/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2026 Jon Evans <jon@craftyjon.com>
 * Copyright The KiCad Developers, see AUTHORS.txt for contributors.
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <api/api_plugin_manager.h>
#include <api/common/types/wizards.pb.h>
#include <pgm_base.h>

#include <google/protobuf/util/json_util.h>

#include "footprint_wizard.h"


void FOOTPRINT_WIZARD_MANAGER::ReloadWizards()
{
    m_wizards.clear();

    API_PLUGIN_MANAGER& manager = Pgm().GetPluginManager();
    std::vector<const PLUGIN_ACTION*> actions = manager.GetActionsForScope( PLUGIN_ACTION_SCOPE::FOOTPRINT_WIZARD );

    for( const PLUGIN_ACTION* action : actions )
    {
        std::unique_ptr<FOOTPRINT_WIZARD> wizard = std::make_unique<FOOTPRINT_WIZARD>();
        wizard->SetIdentifier( action->identifier );

        if( RefreshInfo( wizard.get() ) )
            m_wizards[wizard->Identifier()] = std::move( wizard );
    }
}


std::vector<FOOTPRINT_WIZARD*> FOOTPRINT_WIZARD_MANAGER::Wizards() const
{
    std::vector<FOOTPRINT_WIZARD*> wizards;

    for( const std::unique_ptr<FOOTPRINT_WIZARD>& wizard : m_wizards | std::views::values )
        wizards.emplace_back( wizard.get() );

    std::ranges::sort( wizards,
                       []( FOOTPRINT_WIZARD* const& lhs,
                           FOOTPRINT_WIZARD* const& rhs ) -> bool
                       {
                           if( !lhs || !rhs )
                               return false;

                           return lhs->Info().meta.name < rhs->Info().meta.name;
                       } );

    return wizards;
}


std::optional<FOOTPRINT_WIZARD*> FOOTPRINT_WIZARD_MANAGER::GetWizard( const wxString& aIdentifier )
{
    if( m_wizards.contains( aIdentifier ) )
        return m_wizards[aIdentifier].get();

    return std::nullopt;
}


bool FOOTPRINT_WIZARD_MANAGER::RefreshInfo( FOOTPRINT_WIZARD* aWizard )
{
    wxCHECK( aWizard, false );
    API_PLUGIN_MANAGER& manager = Pgm().GetPluginManager();

    wxString out, err;
    int ret = manager.InvokeActionSync( aWizard->Identifier(), { wxS( "--get-info" ) }, &out, &err );

    if( ret != 0 )
        return false;

    kiapi::common::types::WizardInfo info;

    google::protobuf::util::JsonParseOptions options;
    options.ignore_unknown_fields = true;

    if( !google::protobuf::util::JsonStringToMessage( out.ToStdString(), &info, options ).ok() )
        return false;

    aWizard->Info().FromProto( info );
    return true;
}


void WIZARD_META_INFO::FromProto( const kiapi::common::types::WizardMetaInfo& aProto )
{
    identifier = wxString::FromUTF8( aProto.identifier() );
    name = wxString::FromUTF8( aProto.name() );
    description = wxString::FromUTF8( aProto.description() );

    types_generated.clear();

    for( int type : aProto.types_generated() )
        types_generated.insert( static_cast<kiapi::common::types::WizardContentType>( type ) );
}


wxString WIZARD_PARAMETER::ParameterCategoryName( kiapi::common::types::WizardParameterCategory aCategory )
{
    using namespace kiapi::common::types;

    switch( aCategory )
    {
    case WPC_PACKAGE:   return _( "Package" );
    case WPC_PADS:      return _( "Pads" );
    case WPC_3DMODEL:   return _( "3D Model" );
    case WPC_METADATA:  return _( "General" );
    case WPC_RULES:     return _( "Design Rules" );
    case WPC_UNKNOWN:
    default:
        wxCHECK_MSG( false, wxEmptyString, "Unhandled parameter category type!" );
    }
}


std::unique_ptr<WIZARD_PARAMETER> WIZARD_PARAMETER::Create( const kiapi::common::types::WizardParameter& aProto )
{
    std::unique_ptr<WIZARD_PARAMETER> p;

    if( aProto.has_int_() )
    {
        p = std::make_unique<WIZARD_INT_PARAMETER>();
        static_cast<WIZARD_INT_PARAMETER*>( p.get() )->FromProto( aProto.int_() );
    }
    else if( aProto.has_real() )
    {
        p = std::make_unique<WIZARD_REAL_PARAMETER>();
        static_cast<WIZARD_REAL_PARAMETER*>( p.get() )->FromProto( aProto.real() );
    }
    else if( aProto.has_string() )
    {
        p = std::make_unique<WIZARD_STRING_PARAMETER>();
        static_cast<WIZARD_STRING_PARAMETER*>( p.get() )->FromProto( aProto.string() );
    }
    else if( aProto.has_bool_() )
    {
        p = std::make_unique<WIZARD_BOOL_PARAMETER>();
        static_cast<WIZARD_BOOL_PARAMETER*>( p.get() )->FromProto( aProto.bool_() );
    }

    p->identifier = wxString::FromUTF8( aProto.identifier() );
    p->name = wxString::FromUTF8( aProto.name() );
    p->description = wxString::FromUTF8( aProto.description() );
    p->category = aProto.category();
    p->type = aProto.type();

    return p;
}


void WIZARD_INT_PARAMETER::FromProto( const kiapi::common::types::WizardIntParameter& aProto )
{
    value = aProto.value();
    default_value = aProto.default_();

    if( aProto.has_min() )
        min = aProto.min();
    else
        min.reset();

    if( aProto.has_max() )
        max = aProto.max();
    else
        max.reset();

    if( aProto.has_multiple() )
        multiple = aProto.multiple();
    else
        multiple.reset();
}


void WIZARD_REAL_PARAMETER::FromProto( const kiapi::common::types::WizardRealParameter& aProto )
{
    value = aProto.value();
    default_value = aProto.default_();

    if( aProto.has_min() )
        min = aProto.min();
    else
        min.reset();

    if( aProto.has_max() )
        max = aProto.max();
    else
        max.reset();
}


void WIZARD_BOOL_PARAMETER::FromProto( const kiapi::common::types::WizardBoolParameter& aProto )
{
    value = aProto.value();
    default_value = aProto.default_();
}


void WIZARD_STRING_PARAMETER::FromProto( const kiapi::common::types::WizardStringParameter& aProto )
{
    value = wxString::FromUTF8( aProto.value() );
    default_value = wxString::FromUTF8( aProto.default_() );

    if( aProto.has_validation_regex() )
        validation_regex = wxString::FromUTF8( aProto.validation_regex() );
    else
        validation_regex.reset();
}


void WIZARD_INFO::FromProto( const kiapi::common::types::WizardInfo& aProto )
{
    meta.FromProto( aProto.meta() );

    parameters.clear();
    parameters.reserve( aProto.parameters_size() );

    for( const kiapi::common::types::WizardParameter& parameter : aProto.parameters() )
        parameters.emplace_back( WIZARD_PARAMETER::Create( parameter ) );
}
