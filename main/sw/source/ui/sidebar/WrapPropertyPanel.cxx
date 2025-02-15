/**************************************************************
*
* Licensed to the Apache Software Foundation (ASF) under one
* or more contributor license agreements.  See the NOTICE file
* distributed with this work for additional information
* regarding copyright ownership.  The ASF licenses this file
* to you under the Apache License, Version 2.0 (the
* "License"); you may not use this file except in compliance
* with the License.  You may obtain a copy of the License at
*
*   http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing,
* software distributed under the License is distributed on an
* "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
* KIND, either express or implied.  See the License for the
* specific language governing permissions and limitations
* under the License.
*
*************************************************************/



#include "precompiled_sw.hxx"

#include "WrapPropertyPanel.hxx"
#include "WrapPropertyPanel.hrc"
#include "PropertyPanel.hrc"

#include <cmdid.h>
#include <swtypes.hxx>

#include <sfx2/bindings.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/sidebar/ControlFactory.hxx>
#include <sfx2/imagemgr.hxx>
#include <svl/eitem.hxx>
#include <vcl/svapp.hxx>

#include "com/sun/star/lang/IllegalArgumentException.hpp"

#define A2S(pString) (::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(pString)))


namespace sw { namespace sidebar {

WrapPropertyPanel* WrapPropertyPanel::Create (
	Window* pParent,
	const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& rxFrame,
	SfxBindings* pBindings)
{
	if (pParent == NULL)
		throw ::com::sun::star::lang::IllegalArgumentException(A2S("no parent Window given to WrapPropertyPanel::Create"), NULL, 0);
	if ( ! rxFrame.is())
		throw ::com::sun::star::lang::IllegalArgumentException(A2S("no XFrame given to WrapPropertyPanel::Create"), NULL, 1);
	if (pBindings == NULL)
		throw ::com::sun::star::lang::IllegalArgumentException(A2S("no SfxBindings given to WrapPropertyPanel::Create"), NULL, 2);

	return new WrapPropertyPanel(
		pParent,
		rxFrame,
		pBindings);
}


WrapPropertyPanel::WrapPropertyPanel(
	Window* pParent,
	const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& rxFrame,
	SfxBindings* pBindings )
	: Control(pParent, SW_RES(RID_PROPERTYPANEL_SWOBJWRAP_PAGE))
	, mxFrame( rxFrame )
	, mpBindings(pBindings)
	// visible controls
	, mpRBNoWrap( ::sfx2::sidebar::ControlFactory::CreateCustomImageRadionButton( this, SW_RES(RB_NO_WRAP) ) )
	, mpRBWrapLeft( ::sfx2::sidebar::ControlFactory::CreateCustomImageRadionButton( this, SW_RES(RB_WRAP_LEFT) ) )
	, mpRBWrapRight( ::sfx2::sidebar::ControlFactory::CreateCustomImageRadionButton( this, SW_RES(RB_WRAP_RIGHT) ) )
	, mpRBWrapParallel( ::sfx2::sidebar::ControlFactory::CreateCustomImageRadionButton( this, SW_RES(RB_WRAP_PARALLEL) ) )
	, mpRBWrapThrough( ::sfx2::sidebar::ControlFactory::CreateCustomImageRadionButton( this, SW_RES(RB_WRAP_THROUGH) ) )
	, mpRBIdealWrap( ::sfx2::sidebar::ControlFactory::CreateCustomImageRadionButton( this, SW_RES(RB_WRAP_IDEAL) ) )
	// resources
	, aWrapIL(6,2)
	, aWrapILH(6,2)
	// controller items
	, maSwNoWrapControl(FN_FRAME_NOWRAP, *pBindings, *this)
	, maSwWrapLeftControl(FN_FRAME_WRAP, *pBindings, *this)
	, maSwWrapRightControl(FN_FRAME_WRAP_RIGHT, *pBindings, *this)
	, maSwWrapParallelControl(FN_FRAME_WRAP_LEFT, *pBindings, *this)
	, maSwWrapThroughControl(FN_FRAME_WRAPTHRU, *pBindings, *this)
	, maSwWrapIdealControl(FN_FRAME_WRAP_IDEAL, *pBindings, *this)
{
	Initialize();
	FreeResource();
}


WrapPropertyPanel::~WrapPropertyPanel()
{
}


void WrapPropertyPanel::Initialize()
{
	Link aLink = LINK(this, WrapPropertyPanel, WrapTypeHdl);
	mpRBNoWrap->SetClickHdl(aLink);
	mpRBWrapLeft->SetClickHdl(aLink);
	mpRBWrapRight->SetClickHdl(aLink);
	mpRBWrapParallel->SetClickHdl(aLink);
	mpRBWrapThrough->SetClickHdl(aLink);
	mpRBIdealWrap->SetClickHdl(aLink);

	aWrapIL.AddImage( IMG_NONE,
					  ::GetImage( mxFrame, A2S(".uno:WrapOff"), sal_False, sal_False ) );
	aWrapIL.AddImage( IMG_LEFT,
					  ::GetImage( mxFrame, A2S(".uno:WrapLeft"), sal_False, sal_False ) );
	aWrapIL.AddImage( IMG_RIGHT,
					  ::GetImage( mxFrame, A2S(".uno:WrapRight"), sal_False, sal_False ) );
	aWrapIL.AddImage( IMG_PARALLEL,
					  ::GetImage( mxFrame, A2S(".uno:WrapOn"), sal_False, sal_False ) );
	aWrapIL.AddImage( IMG_THROUGH,
					  ::GetImage( mxFrame, A2S(".uno:WrapThrough"), sal_False, sal_False ) );
	aWrapIL.AddImage( IMG_IDEAL,
					  ::GetImage( mxFrame, A2S(".uno:WrapIdeal"), sal_False, sal_False ) );

	aWrapILH.AddImage( IMG_NONE,
					   ::GetImage( mxFrame, A2S(".uno:WrapOff"), sal_False, sal_True ) );
	aWrapILH.AddImage( IMG_LEFT,
					   ::GetImage( mxFrame, A2S(".uno:WrapLeft"), sal_False, sal_True ) );
	aWrapILH.AddImage( IMG_RIGHT,
					   ::GetImage( mxFrame, A2S(".uno:WrapRight"), sal_False, sal_True ) );
	aWrapILH.AddImage( IMG_PARALLEL,
					   ::GetImage( mxFrame, A2S(".uno:WrapOn"), sal_False, sal_True ) );
	aWrapILH.AddImage( IMG_THROUGH,
					   ::GetImage( mxFrame, A2S(".uno:WrapThrough"), sal_False, sal_True ) );
	aWrapILH.AddImage( IMG_IDEAL,
					   ::GetImage( mxFrame, A2S(".uno:WrapIdeal"), sal_False, sal_True ) );

	mpRBNoWrap->SetModeRadioImage( aWrapIL.GetImage(IMG_NONE) );
	mpRBNoWrap->SetModeRadioImage( aWrapILH.GetImage(IMG_NONE) , BMP_COLOR_HIGHCONTRAST );
	if ( Application::GetSettings().GetLayoutRTL() )
	{
		mpRBWrapLeft->SetModeRadioImage( aWrapIL.GetImage(IMG_RIGHT) );
		mpRBWrapLeft->SetModeRadioImage( aWrapILH.GetImage(IMG_RIGHT) , BMP_COLOR_HIGHCONTRAST );
		mpRBWrapRight->SetModeRadioImage( aWrapIL.GetImage(IMG_LEFT) );
		mpRBWrapRight->SetModeRadioImage( aWrapILH.GetImage(IMG_LEFT) , BMP_COLOR_HIGHCONTRAST );
	}
	else
	{
		mpRBWrapLeft->SetModeRadioImage( aWrapIL.GetImage(IMG_LEFT) );
		mpRBWrapLeft->SetModeRadioImage( aWrapILH.GetImage(IMG_LEFT) , BMP_COLOR_HIGHCONTRAST );
		mpRBWrapRight->SetModeRadioImage( aWrapIL.GetImage(IMG_RIGHT) );
		mpRBWrapRight->SetModeRadioImage( aWrapILH.GetImage(IMG_RIGHT) , BMP_COLOR_HIGHCONTRAST );
	}
	mpRBWrapParallel->SetModeRadioImage( aWrapIL.GetImage(IMG_PARALLEL) );
	mpRBWrapParallel->SetModeRadioImage( aWrapILH.GetImage(IMG_PARALLEL) , BMP_COLOR_HIGHCONTRAST );
	mpRBWrapThrough->SetModeRadioImage( aWrapIL.GetImage(IMG_THROUGH) );
	mpRBWrapThrough->SetModeRadioImage( aWrapILH.GetImage(IMG_THROUGH) , BMP_COLOR_HIGHCONTRAST );
	mpRBIdealWrap->SetModeRadioImage( aWrapIL.GetImage(IMG_IDEAL) );
	mpRBIdealWrap->SetModeRadioImage( aWrapILH.GetImage(IMG_IDEAL) , BMP_COLOR_HIGHCONTRAST );

	mpRBNoWrap->SetAccessibleName(mpRBNoWrap->GetQuickHelpText());
	mpRBWrapLeft->SetAccessibleName(mpRBWrapLeft->GetQuickHelpText());
	mpRBWrapRight->SetAccessibleName(mpRBWrapRight->GetQuickHelpText());
	mpRBWrapParallel->SetAccessibleName(mpRBWrapParallel->GetQuickHelpText());
	mpRBWrapThrough->SetAccessibleName(mpRBWrapThrough->GetQuickHelpText());
	mpRBIdealWrap->SetAccessibleName(mpRBIdealWrap->GetQuickHelpText());

	mpBindings->Update( FN_FRAME_NOWRAP );
	mpBindings->Update( FN_FRAME_WRAP );
	mpBindings->Update( FN_FRAME_WRAP_RIGHT );
	mpBindings->Update( FN_FRAME_WRAP_LEFT );
	mpBindings->Update( FN_FRAME_WRAPTHRU );
	mpBindings->Update( FN_FRAME_WRAP_IDEAL );
}


IMPL_LINK(WrapPropertyPanel, WrapTypeHdl, void *, EMPTYARG)
{
	sal_uInt16 nSlot = 0;
	if ( mpRBWrapLeft->IsChecked() )
	{
		nSlot = FN_FRAME_WRAP_LEFT;
	}
	else if( mpRBWrapRight->IsChecked() )
	{
		nSlot = FN_FRAME_WRAP_RIGHT;
	}
	else if ( mpRBWrapParallel->IsChecked() )
	{
		nSlot = FN_FRAME_WRAP;
	}
	else if( mpRBWrapThrough->IsChecked() )
	{
		nSlot = FN_FRAME_WRAPTHRU;
	}
	else if( mpRBIdealWrap->IsChecked() )
	{
		nSlot = FN_FRAME_WRAP_IDEAL;
	}
	else
	{
		nSlot = FN_FRAME_NOWRAP;
	}
	SfxBoolItem bStateItem( nSlot, sal_True );
	mpBindings->GetDispatcher()->Execute( nSlot, SFX_CALLMODE_RECORD, &bStateItem, 0L );

	return 0;
}


void WrapPropertyPanel::NotifyItemUpdate(
	const sal_uInt16 nSId,
	const SfxItemState eState,
	const SfxPoolItem* pState,
	const bool bIsEnabled)
{
	(void)bIsEnabled;

	if ( eState == SFX_ITEM_AVAILABLE &&
		pState->ISA(SfxBoolItem) )
	{
		// Set Radio Button enable
		mpRBNoWrap->Enable(true);
		mpRBWrapLeft->Enable(true);
		mpRBWrapRight->Enable(true);
		mpRBWrapParallel->Enable(true);
		mpRBWrapThrough->Enable(true);
		mpRBIdealWrap->Enable(true);

		const SfxBoolItem* pBoolItem = static_cast< const SfxBoolItem* >( pState );
		switch( nSId )
		{
		case FN_FRAME_WRAP_RIGHT:
			mpRBWrapRight->Check( pBoolItem->GetValue() );
			break;
		case FN_FRAME_WRAP_LEFT:
			mpRBWrapLeft->Check( pBoolItem->GetValue() );
			break;
		case FN_FRAME_WRAPTHRU:
			mpRBWrapThrough->Check( pBoolItem->GetValue() );
			break;
		case FN_FRAME_WRAP_IDEAL:
			mpRBIdealWrap->Check( pBoolItem->GetValue() );
			break;
		case FN_FRAME_WRAP:
			mpRBWrapParallel->Check( pBoolItem->GetValue() );
			break;
		case FN_FRAME_NOWRAP:
		default:
			mpRBNoWrap->Check( pBoolItem->GetValue() );
			break;
		}
	}
	else
	{
		mpRBNoWrap->Enable(false);
		mpRBWrapLeft->Enable(false);
		mpRBWrapRight->Enable(false);
		mpRBWrapParallel->Enable(false);
		mpRBWrapThrough->Enable(false);
		mpRBIdealWrap->Enable(false);

		mpRBNoWrap->Check( sal_False );
		mpRBWrapLeft->Check( sal_False );
		mpRBWrapRight->Check( sal_False );
		mpRBWrapParallel->Check( sal_False );
		mpRBWrapThrough->Check( sal_False );
		mpRBIdealWrap->Check( sal_False );
	}
}

} } // end of namespace ::sw::sidebar

/* vim: set noet sw=4 ts=4: */
