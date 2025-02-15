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



#include "precompiled_svx.hxx"
#ifndef _NBDTMG_HXX
#include <svx/nbdtmg.hxx>
#endif
#ifndef _SVX_SVXIDS_HRC
#include <svx/svxids.hrc>
#endif
#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif
#ifndef _SFXITEMSET_HXX
#include <svl/itemset.hxx>
#endif
#ifndef _SFXREQUEST_HXX
#include <sfx2/request.hxx>
#endif
#ifndef _SFXSTRITEM_HXX
#include <svl/stritem.hxx>
#endif
#ifndef _UNO_LINGU_HXX
#include <editeng/unolingu.hxx>
#endif
#ifndef _CTRLTOOL_HXX
#include <svtools/ctrltool.hxx>
#endif
#ifndef _SFX_OBJSH_HXX
#include <sfx2/objsh.hxx>
#endif
#ifndef _SVX_FLSTITEM_HXX
#include <editeng/flstitem.hxx>
#endif
#ifndef _SFXITEMPOOL_HXX
#include <svl/itempool.hxx>
#endif
#ifndef _SV_OUTDEV_HXX
#include <vcl/outdev.hxx>
#endif
#ifndef _GALLERY_HXX_
#include <svx/gallery.hxx>
#endif
#ifndef _SVX_BRSHITEM_HXX
#include <editeng/brshitem.hxx>
#endif
#include <svx/dialmgr.hxx>
#ifndef _SVX_DIALOGS_HRC
#include <svx/dialogs.hrc>
#endif
#ifndef _SV_GRAPH_HXX
#include <vcl/graph.hxx>
#endif

#include <unotools/streamwrap.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <unotools/pathoptions.hxx>
#include <editeng/eeitem.hxx>

#include <com/sun/star/text/HoriOrientation.hpp>
#include <com/sun/star/text/VertOrientation.hpp>
#include <com/sun/star/text/RelOrientation.hpp>
#include <com/sun/star/style/NumberingType.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/text/XDefaultNumberingProvider.hpp>
#include <com/sun/star/text/XNumberingFormatter.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <comphelper/processfactory.hxx>
#include <com/sun/star/text/XNumberingTypeInfo.hpp>

using namespace com::sun::star;
using namespace com::sun::star::uno;
using namespace com::sun::star::beans;
using namespace com::sun::star::lang;
using namespace com::sun::star::text;
using namespace com::sun::star::container;
using namespace com::sun::star::style;
using rtl::OUString;

namespace svx { namespace sidebar {
#define NUM_TYPE_MEMBER			4
#define NUM_VALUSET_COUNT		16
#define MAX_VALUESET_GRAPHIC	30

typedef NumSettings_Impl* NumSettings_ImplPtr;
SV_DECL_PTRARR_DEL(NumSettingsArr_Impl,NumSettings_ImplPtr,8,4)
SV_IMPL_PTRARR( NumSettingsArr_Impl, NumSettings_ImplPtr )

typedef NumberSettings_Impl* NumberSettings_ImplPtr;
SV_DECL_PTRARR_DEL(NumberSettingsArr_Impl,NumberSettings_ImplPtr,8,4)
SV_IMPL_PTRARR( NumberSettingsArr_Impl, NumberSettings_ImplPtr )

Font& lcl_GetDefaultBulletFont()
{
	static sal_Bool bInit = 0;
	static Font aDefBulletFont( UniString::CreateFromAscii(
								RTL_CONSTASCII_STRINGPARAM( "StarSymbol" ) ),
								String(), Size( 0, 14 ) );
	if(!bInit)
	{
		aDefBulletFont.SetCharSet( RTL_TEXTENCODING_SYMBOL );
		aDefBulletFont.SetFamily( FAMILY_DONTKNOW );
		aDefBulletFont.SetPitch( PITCH_DONTKNOW );
		aDefBulletFont.SetWeight( WEIGHT_DONTKNOW );
		aDefBulletFont.SetTransparent( sal_True );
		bInit = sal_True;
	}
	return aDefBulletFont;
}

static const sal_Unicode aDefaultBulletTypes[] =
{
	0x2022,
	0x25cf,
	0xe00c,
	0xe00a,
	0x2794,
	0x27a2,
	0x2717,
	0x2714
};

static const sal_Unicode aDefaultRTLBulletTypes[] =
{
	0x2022,
	0x25cf,
	0xe00c,
	0xe00a,
	0x25c4,
	0x272b,
	0x2717,
	0x2714
};

static const sal_Char sNumberingType[] = "NumberingType";
static const sal_Char sValue[] = "Value";
static const sal_Char sParentNumbering[] = "ParentNumbering";
static const sal_Char sPrefix[] = "Prefix";
static const sal_Char sSuffix[] = "Suffix";
static const sal_Char sBulletChar[] = "BulletChar";
static const sal_Char sBulletFontName[] = "BulletFontName";

NumSettings_ImplPtr lcl_CreateNumberingSettingsPtr(const Sequence<PropertyValue>& rLevelProps)
{
	const PropertyValue* pValues = rLevelProps.getConstArray();
	NumSettings_ImplPtr pNew = new NumSettings_Impl;
	for(sal_Int32 j = 0; j < rLevelProps.getLength(); j++)
	{
		if(pValues[j].Name.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM(sNumberingType)))
			pValues[j].Value >>= pNew->nNumberType;
		else if(pValues[j].Name.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM(sPrefix)))
			pValues[j].Value >>= pNew->sPrefix;
		else if(pValues[j].Name.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM(sSuffix)))
			pValues[j].Value >>= pNew->sSuffix;
		else if(pValues[j].Name.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM(sParentNumbering)))
			pValues[j].Value >>= pNew->nParentNumbering;
		else if(pValues[j].Name.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM(sBulletChar)))
			pValues[j].Value >>= pNew->sBulletChar;
		else if(pValues[j].Name.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM(sBulletFontName)))
			pValues[j].Value >>= pNew->sBulletFont;
	}
	const sal_Unicode cLocalPrefix = pNew->sPrefix.getLength() ? pNew->sPrefix.getStr()[0] : 0;
	const sal_Unicode cLocalSuffix = pNew->sSuffix.getLength() ? pNew->sSuffix.getStr()[0] : 0;
	String aEmptyStr;
	if( cLocalPrefix == ' ') pNew->sPrefix=aEmptyStr;
	if( cLocalSuffix == ' ') pNew->sSuffix=aEmptyStr;
	return pNew;
}

sal_uInt16 NBOTypeMgrBase:: IsSingleLevel(sal_uInt16 nCurLevel)
{
	sal_uInt16 nLv = (sal_uInt16)0xFFFF;
	sal_uInt16 nCount = 0;
	sal_uInt16 nMask = 1;
	for( sal_uInt16 i = 0; i < SVX_MAX_NUM; i++ )
	{
		if(nCurLevel & nMask)
		{
			nCount++;
			nLv=i;
		}
		nMask <<= 1 ;
	}

	if ( nCount == 1)
		return nLv;
	else
		return (sal_uInt16)0xFFFF;
}

void NBOTypeMgrBase::StoreBulCharFmtName_impl() {
		if ( pSet )
		{
			SfxAllItemSet aSet(*pSet);
			SFX_ITEMSET_ARG(&aSet,pBulletCharFmt,SfxStringItem,SID_BULLET_CHAR_FMT,sal_False);

			if ( pBulletCharFmt )
			{
				aNumCharFmtName = String(pBulletCharFmt->GetValue());
			}
		}
}
String NBOTypeMgrBase::GetBulCharFmtName()
{
	return aNumCharFmtName;
}
void NBOTypeMgrBase::ImplLoad(String filename)
{
	bIsLoading = true;
	SfxMapUnit		eOldCoreUnit=eCoreUnit;
	eCoreUnit = SFX_MAPUNIT_100TH_MM;
	INetURLObject aFile( SvtPathOptions().GetPalettePath() );
	aFile.Append( filename);
	SvStream* pIStm = ::utl::UcbStreamHelper::CreateStream( aFile.GetMainURL( INetURLObject::NO_DECODE ), STREAM_READ );
	if( pIStm ) {
		sal_uInt32	nVersion;
		sal_Int32	nNumIndex;
		*pIStm >> nVersion;
		if (nVersion==DEFAULT_NUMBERING_CACHE_FORMAT_VERSION) // first version
		{
			*pIStm >> nNumIndex;
			sal_uInt16 mLevel = 0x1;
			while (nNumIndex>=0 && nNumIndex<DEFAULT_NUM_VALUSET_COUNT) {
				SvxNumRule aNum(*pIStm);
				// bullet color in font properties is not stored correctly. Need set transparency bits manually
				for(sal_uInt16 i = 0; i < aNum.GetLevelCount(); i++)
				{
					SvxNumberFormat aFmt(aNum.GetLevel(i));
					if (aFmt.GetBulletFont()) {
						Font aFont(*aFmt.GetBulletFont());
						Color c=aFont.GetColor();
						c.SetTransparency(0xFF);
						aFont.SetColor(c);
						aFmt.SetBulletFont(&aFont);
						aNum.SetLevel(i, aFmt);
					}
				}
				RelplaceNumRule(aNum,nNumIndex,mLevel);
				*pIStm >> nNumIndex;
			}
			delete pIStm;
		}
	}
	eCoreUnit = eOldCoreUnit;
	bIsLoading = false;
}
void NBOTypeMgrBase::ImplStore(String filename)
{
	if (bIsLoading) return;
	SfxMapUnit		eOldCoreUnit=eCoreUnit;
	eCoreUnit = SFX_MAPUNIT_100TH_MM;
	INetURLObject aFile( SvtPathOptions().GetPalettePath() );
	aFile.Append( filename);
	SvStream* pOStm = ::utl::UcbStreamHelper::CreateStream( aFile.GetMainURL( INetURLObject::NO_DECODE ), STREAM_WRITE );
	if( pOStm ) {
		sal_uInt32	nVersion;
		sal_Int32	nNumIndex;
		nVersion = DEFAULT_NUMBERING_CACHE_FORMAT_VERSION;
		*pOStm << nVersion;
		for(sal_Int32 nItem = 0; nItem < DEFAULT_NUM_VALUSET_COUNT; nItem++ ) {
			if (IsCustomized(nItem)) {
				SvxNumRule aDefNumRule( NUM_BULLET_REL_SIZE|NUM_CONTINUOUS|NUM_BULLET_COLOR|NUM_CHAR_TEXT_DISTANCE|NUM_SYMBOL_ALIGNMENT,10, sal_False ,
					SVX_RULETYPE_NUMBERING,SvxNumberFormat::LABEL_ALIGNMENT);
				sal_uInt16 mLevel = 0x1;
				*pOStm << nItem;
				ApplyNumRule(aDefNumRule,nItem,mLevel,false,true);
				aDefNumRule.Store(*pOStm);
			}
		}
		nNumIndex = -1;
		*pOStm << nNumIndex; // write end flag
		delete pOStm;
	}
	eCoreUnit = eOldCoreUnit;
}

void NBOTypeMgrBase::StoreMapUnit_impl() {
	if ( pSet )
	{
		const SfxPoolItem* pItem;
		SfxItemState eState = pSet->GetItemState(SID_ATTR_NUMBERING_RULE, sal_False, &pItem);
		if(eState == SFX_ITEM_SET)
		{
			eCoreUnit = pSet->GetPool()->GetMetric(pSet->GetPool()->GetWhich(SID_ATTR_NUMBERING_RULE));
		} else {
			// sd use different sid for numbering rule
			eState = pSet->GetItemState(EE_PARA_NUMBULLET, sal_False, &pItem);
			if(eState == SFX_ITEM_SET)
			{
				eCoreUnit = pSet->GetPool()->GetMetric(pSet->GetPool()->GetWhich(EE_PARA_NUMBULLET));
			}
		}
	}
}
SfxMapUnit NBOTypeMgrBase::GetMapUnit()
{
	return eCoreUnit;
}
/***************************************************************************************************
**********************Character Bullet Type lib**********************************************************
****************************************************************************************************/
BulletsTypeMgr* BulletsTypeMgr::_instance = 0;
BulletsSettings_Impl* BulletsTypeMgr::pActualBullets[] ={0,0,0,0,0,0,0,0};
sal_Unicode BulletsTypeMgr::aDynamicBulletTypes[]={' ',' ',' ',' ',' ',' ',' ',' '};
sal_Unicode BulletsTypeMgr::aDynamicRTLBulletTypes[]={' ',' ',' ',' ',' ',' ',' ',' '};

BulletsTypeMgr::BulletsTypeMgr(const NBOType aType):
	NBOTypeMgrBase(aType)
{
	Init();
}

BulletsTypeMgr::BulletsTypeMgr(const NBOType aType,const SfxItemSet* pArg):
	NBOTypeMgrBase(aType,pArg)
{
	Init();
}

BulletsTypeMgr::BulletsTypeMgr(const BulletsTypeMgr& aTypeMgr):
	NBOTypeMgrBase(aTypeMgr)
{
	for (sal_uInt16 i=0;i<DEFAULT_BULLET_TYPES;i++)
	{
		pActualBullets[i]->bIsCustomized = aTypeMgr.pActualBullets[i]->bIsCustomized;
		pActualBullets[i]->cBulletChar = aTypeMgr.pActualBullets[i]->cBulletChar;
		pActualBullets[i]->aFont = aTypeMgr.pActualBullets[i]->aFont;
		pActualBullets[i]->sDescription = aTypeMgr. pActualBullets[i]->sDescription;
		pActualBullets[i]->eType = aTypeMgr. pActualBullets[i]->eType;
	}
}
void BulletsTypeMgr::Init()
{
	Font& rActBulletFont = lcl_GetDefaultBulletFont();
	String sName = rActBulletFont.GetName();
	if( Application::GetSettings().GetLayoutRTL() )
	{
		for (sal_uInt16 i=0;i<DEFAULT_BULLET_TYPES;i++)
		{
			pActualBullets[i] = new BulletsSettings_Impl(eNBType::BULLETS);
			pActualBullets[i]->cBulletChar = aDefaultRTLBulletTypes[i];
			pActualBullets[i]->aFont = rActBulletFont;
			if (i==4 || i==5)
				pActualBullets[i]->sDescription = SVX_RESSTR( RID_SVXSTR_BULLET_RTL_DESCRIPTION_4 - 4 + i );
			else
				pActualBullets[i]->sDescription = SVX_RESSTR( RID_SVXSTR_BULLET_DESCRIPTION_0 + i );
		}
	}else
	{
		for (sal_uInt16 i=0;i<DEFAULT_BULLET_TYPES;i++)
		{
			pActualBullets[i] = new BulletsSettings_Impl(eNBType::BULLETS);
			pActualBullets[i]->cBulletChar = aDefaultBulletTypes[i];
			pActualBullets[i]->aFont =rActBulletFont;
			pActualBullets[i]->sDescription = SVX_RESSTR( RID_SVXSTR_BULLET_DESCRIPTION_0 + i );
		}
	}
}
sal_uInt16 BulletsTypeMgr::GetNBOIndexForNumRule(SvxNumRule& aNum,sal_uInt16 mLevel,sal_uInt16 nFromIndex)
{
	if ( mLevel == (sal_uInt16)0xFFFF || mLevel == 0)
		return (sal_uInt16)0xFFFF;
	//if ( !lcl_IsNumFmtSet(pNR, mLevel) ) return (sal_uInt16)0xFFFF;

	sal_uInt16 nActLv = IsSingleLevel(mLevel);

	if ( nActLv == (sal_uInt16)0xFFFF )
		return (sal_uInt16)0xFFFF;

	SvxNumberFormat aFmt(aNum.GetLevel(nActLv));
	sal_Unicode cChar = aFmt.GetBulletChar();
	//sal_uInt16 nLength = 0;
	/*if( Application::GetSettings().GetLayoutRTL() )
	{
		nLength = sizeof(aDynamicRTLBulletTypes)/sizeof(sal_Unicode);
		for(sal_uInt16 i = 0; i < nLength; i++)
		{
			if ( cChar == aDynamicRTLBulletTypes[i] ||
				(cChar == 9830 && 57356 == aDynamicRTLBulletTypes[i]) ||
				(cChar == 9632 && 57354 == aDynamicRTLBulletTypes[i]) )
			{
				return i+1;
			}
		}
	} else
	{
		nLength = sizeof(aDynamicBulletTypes)/sizeof(sal_Unicode);
		for(sal_uInt16 i = 0; i < nLength; i++)
		{
			if ( cChar == aDynamicBulletTypes[i] ||
				(cChar == 9830 && 57356 == aDynamicBulletTypes[i]) ||
				(cChar == 9632 && 57354 == aDynamicBulletTypes[i]) )
			{
				return i+1;
			}
		}
	}*/
	//nLength = sizeof(pActualBullets)/sizeof(BulletsSettings_Impl);
	for(sal_uInt16 i = nFromIndex; i < DEFAULT_BULLET_TYPES; i++)
	{
		if ( (cChar == pActualBullets[i]->cBulletChar||
			(cChar == 9830 && 57356 == pActualBullets[i]->cBulletChar) ||
			(cChar == 9632 && 57354 == pActualBullets[i]->cBulletChar)))// && pFont && (pFont->GetName().CompareTo(pActualBullets[i]->aFont.GetName())==COMPARE_EQUAL))
		{
			return i+1;
		}
	}

	return (sal_uInt16)0xFFFF;
}

sal_Bool BulletsTypeMgr::RelplaceNumRule(SvxNumRule& aNum,sal_uInt16 nIndex,sal_uInt16 mLevel)
{
	if ( mLevel == (sal_uInt16)0xFFFF || mLevel == 0)
		return sal_False;

	if ( GetNBOIndexForNumRule(aNum,mLevel) != (sal_uInt16)0xFFFF )
		return sal_False;

	sal_uInt16 nActLv = IsSingleLevel(mLevel);

	if ( nActLv == (sal_uInt16)0xFFFF )
		return sal_False;

	SvxNumberFormat aFmt(aNum.GetLevel(nActLv));
	sal_Unicode cChar = aFmt.GetBulletChar();
	const Font* pFont = aFmt.GetBulletFont();
	//sal_uInt16 nLength = 0;
	/*if( Application::GetSettings().GetLayoutRTL() )
	{
		nLength = sizeof(aDynamicRTLBulletTypes)/sizeof(sal_Unicode);

		if ( nIndex >= nLength )
			return sal_False;

		aDynamicRTLBulletTypes[nIndex] = cChar;
	} else
	{
		nLength = sizeof(aDynamicBulletTypes)/sizeof(sal_Unicode);

		if ( nIndex >= nLength )
			return sal_False;

		aDynamicBulletTypes[nIndex] = cChar;
	}*/
	//nLength = sizeof(pActualBullets)/sizeof(BulletsSettings_Impl);
	if ( nIndex >= DEFAULT_BULLET_TYPES )
		return sal_False;

	pActualBullets[nIndex]->cBulletChar = cChar;
	if ( pFont )
		pActualBullets[nIndex]->aFont = *pFont;
	pActualBullets[nIndex]->bIsCustomized = sal_True;

	String aStrFromRES = String(SVX_RESSTR( RID_SVXSTR_NUMBULLET_CUSTOM_BULLET_DESCRIPTION));
	String aReplace = String::CreateFromAscii("%LIST_NUM");
	String sNUM = String::CreateFromInt32( nIndex + 1 );
	aStrFromRES.SearchAndReplace(aReplace,sNUM);
	pActualBullets[nIndex]->sDescription = aStrFromRES;

	return sal_True;
}

sal_Bool BulletsTypeMgr::ApplyNumRule(SvxNumRule& aNum,sal_uInt16 nIndex,sal_uInt16 mLevel,sal_Bool /* isDefault */,sal_Bool isResetSize)
{
	//if ( mLevel == (sal_uInt16)0xFFFF )
	//	return sal_False;

	sal_Unicode cChar;
	//sal_uInt16 nLength = 0;
	/*if( Application::GetSettings().GetLayoutRTL() )
	{
		nLength = sizeof(aDynamicRTLBulletTypes)/sizeof(sal_Unicode);

		if ( nIndex >= nLength )
			return sal_False;

		cChar = aDynamicRTLBulletTypes[nIndex];
	}else
	{
		nLength = sizeof(aDynamicBulletTypes)/sizeof(sal_Unicode);

		if ( nIndex >= nLength )
			return sal_False;

		cChar = aDynamicBulletTypes[nIndex];
	}*/
	//nLength = sizeof(pActualBullets)/sizeof(BulletsSettings_Impl);
	if ( nIndex >= DEFAULT_BULLET_TYPES )
		return sal_False;
	cChar = pActualBullets[nIndex]->cBulletChar;
	//Font& rActBulletFont = lcl_GetDefaultBulletFont();
	Font rActBulletFont = pActualBullets[nIndex]->aFont;

	sal_uInt16 nMask = 1;
	String sBulletCharFmtName = GetBulCharFmtName();
	for(sal_uInt16 i = 0; i < aNum.GetLevelCount(); i++)
	{
		if(mLevel & nMask)
		{
			SvxNumberFormat aFmt(aNum.GetLevel(i));
			aFmt.SetNumberingType( SVX_NUM_CHAR_SPECIAL );
			aFmt.SetBulletFont(&rActBulletFont);
			aFmt.SetBulletChar(cChar );
			aFmt.SetCharFmtName(sBulletCharFmtName);
			if (isResetSize) aFmt.SetBulletRelSize(45);
			aNum.SetLevel(i, aFmt);
		}
		nMask <<= 1;
	}

	return sal_True;
}

String BulletsTypeMgr::GetDescription(sal_uInt16 nIndex,sal_Bool /* isDefault */)
{
	String sRet;
	//sal_uInt16 nLength = 0;
	//nLength = sizeof(pActualBullets)/sizeof(BulletsSettings_Impl);

	if ( nIndex >= DEFAULT_BULLET_TYPES )
		return sRet;
	else
		sRet = pActualBullets[nIndex]->sDescription;

	return sRet;
}
sal_Bool BulletsTypeMgr::IsCustomized(sal_uInt16 nIndex)
{
	sal_Bool bRet = sal_False;
	//sal_uInt16 nLength = 0;
	//nLength = sizeof(pActualBullets)/sizeof(BulletsSettings_Impl);

	if ( nIndex >= DEFAULT_BULLET_TYPES )
		bRet = sal_False;
	else
		bRet = pActualBullets[nIndex]->bIsCustomized;

	return bRet;
}

sal_Unicode BulletsTypeMgr::GetBulChar(sal_uInt16 nIndex)
{
	sal_Unicode cChar;
	//sal_uInt16 nLength = 0;
	//nLength = sizeof(pActualBullets)/sizeof(BulletsSettings_Impl);

	if ( nIndex >= DEFAULT_BULLET_TYPES )
		cChar = ' ';
	else
		cChar = pActualBullets[nIndex]->cBulletChar;

	/*if( Application::GetSettings().GetLayoutRTL() )
	{
		nLength = sizeof(aDynamicRTLBulletTypes)/sizeof(sal_Unicode);

		if ( nIndex >= nLength )
			cChar = ' ';
		else
			cChar = aDynamicRTLBulletTypes[nIndex];
	}else
	{
		nLength = sizeof(aDynamicBulletTypes)/sizeof(sal_Unicode);

		if ( nIndex >= nLength )
			cChar = ' ';
		else
			cChar = aDynamicBulletTypes[nIndex];
	}*/

	return cChar;
}
Font BulletsTypeMgr::GetBulCharFont(sal_uInt16 nIndex)
{
	Font aRet;
	if ( nIndex >= DEFAULT_BULLET_TYPES )
		aRet = lcl_GetDefaultBulletFont();
	else
		aRet = pActualBullets[nIndex]->aFont;

	return aRet;
}
/***************************************************************************************************
**********************Graphic Bullet Type lib***********************************************************
****************************************************************************************************/
GraphicBulletsTypeMgr* GraphicBulletsTypeMgr::_instance = 0;
GraphicBulletsTypeMgr::GraphicBulletsTypeMgr(const NBOType aType):
	NBOTypeMgrBase(aType)
{
	Init();
}

GraphicBulletsTypeMgr::GraphicBulletsTypeMgr(const NBOType aType,const SfxItemSet* pArg):
	NBOTypeMgrBase(aType,pArg)
{
	Init();
}
GraphicBulletsTypeMgr::GraphicBulletsTypeMgr(const GraphicBulletsTypeMgr& aTypeMgr):
	NBOTypeMgrBase(aTypeMgr)
{
	for (sal_uInt16 i=0;i< aTypeMgr.aGrfDataLst.Count();i++)
	{
		GrfBulDataRelation* pEntry = new GrfBulDataRelation(eNBType::GRAPHICBULLETS);
		GrfBulDataRelation* pSrcEntry = (GrfBulDataRelation*)(aTypeMgr.aGrfDataLst.GetObject(i));
		if ( pEntry && pSrcEntry)
		{
			pEntry->bIsCustomized = pSrcEntry->bIsCustomized;
			pEntry->nTabIndex = pSrcEntry->nTabIndex;
			pEntry->nGallaryIndex = pSrcEntry->nGallaryIndex;
			pEntry->sGrfName = pSrcEntry->sGrfName;
			pEntry->sDescription = pSrcEntry->sDescription;
			aGrfDataLst.Insert( pEntry, LIST_APPEND );
		}
		else
			delete pEntry;
	}
}
void GraphicBulletsTypeMgr::Init()
{
	List aGrfNames;
	GalleryExplorer::FillObjList(GALLERY_THEME_BULLETS, aGrfNames);
	for(sal_uInt16 i = 0; i < aGrfNames.Count(); i++)
	{
		String* pGrfNm = (String*) aGrfNames.GetObject(i);
			INetURLObject aObj(*pGrfNm);
			if(aObj.GetProtocol() == INET_PROT_FILE)
					*pGrfNm = aObj.PathToFileName();

		GrfBulDataRelation* pEntry = new GrfBulDataRelation(eNBType::GRAPHICBULLETS);
		pEntry->nTabIndex = i+1;
		pEntry->nGallaryIndex = i;
		pEntry->sGrfName = *pGrfNm;

		if( i < MAX_VALUESET_GRAPHIC )
		{
			pEntry->sDescription = SVX_RESSTR( RID_SVXSTR_GRAPHICS_DESCRIPTIONS + i );
		}else
		{
			pEntry->sDescription = *pGrfNm;
		}

		aGrfDataLst.Insert( pEntry, LIST_APPEND );
	}
}
sal_uInt16 GraphicBulletsTypeMgr::GetNBOIndexForNumRule(SvxNumRule& aNum,sal_uInt16 mLevel,sal_uInt16 /* nFromIndex */)
{
	if ( mLevel == (sal_uInt16)0xFFFF || mLevel == 0)
		return (sal_uInt16)0xFFFF;

	sal_uInt16 nActLv = IsSingleLevel(mLevel);

	if ( nActLv == (sal_uInt16)0xFFFF )
		return (sal_uInt16)0xFFFF;

	SvxNumberFormat aFmt(aNum.GetLevel(nActLv));
	const SvxBrushItem* pBrsh = aFmt.GetBrush();
	const Graphic* pGrf = 0;
	if ( pBrsh )
		pGrf = pBrsh->GetGraphic();

	if ( pGrf )
	{
		Graphic aGraphic;
		for(sal_uInt16 i=0;i<aGrfDataLst.Count();i++)
		{
			GrfBulDataRelation* pEntry = (GrfBulDataRelation*) aGrfDataLst.GetObject(i);
			sal_Bool bExist = sal_False;
			if ( pEntry) // && pEntry->sGrfName.CompareTo(*pGrfName)==COMPARE_EQUAL )
				bExist = GalleryExplorer::GetGraphicObj(GALLERY_THEME_BULLETS, pEntry->nGallaryIndex,&aGraphic);
			if (bExist) {
				Bitmap aSum=pGrf->GetBitmap();
				Bitmap aSum1=aGraphic.GetBitmap();
				if (aSum.IsEqual(aSum1))
				return pEntry->nTabIndex;
			}
		}
	}

	return (sal_uInt16)0xFFFF;
}

sal_Bool GraphicBulletsTypeMgr::RelplaceNumRule(SvxNumRule& aNum,sal_uInt16 nIndex,sal_uInt16 mLevel)
{
	if ( mLevel == (sal_uInt16)0xFFFF || mLevel > aNum.GetLevelCount() || mLevel == 0)
		return sal_False;

	if ( GetNBOIndexForNumRule(aNum,mLevel) != (sal_uInt16)0xFFFF )
		return sal_False;

	if ( nIndex >= aGrfDataLst.Count() )
		return sal_False;

	sal_uInt16 nActLv = IsSingleLevel(mLevel);
	if ( nActLv == (sal_uInt16)0xFFFF )
		return sal_False;

	SvxNumberFormat aFmt(aNum.GetLevel(nActLv));
	const SvxBrushItem* pBrsh = aFmt.GetBrush();
	const Graphic* pGrf = 0;
	if ( pBrsh )
		pGrf = pBrsh->GetGraphic();
	else
		return sal_False;

	String sEmpty;
	if ( pGrf )
	{
		const String* pGrfName = pBrsh->GetGraphicLink();
		//String* pGrfName = (String*)(pBrsh->GetGraphicLink());
		GrfBulDataRelation* pEntry = (GrfBulDataRelation*) aGrfDataLst.GetObject(nIndex);
		if ( pGrfName )
			pEntry->sGrfName = *pGrfName;
		//pEntry->sDescription = sEmpty;
		pEntry->nGallaryIndex = (sal_uInt16)0xFFFF;
		pEntry->bIsCustomized = sal_True;
		String aStrFromRES = String(SVX_RESSTR( RID_SVXSTR_NUMBULLET_CUSTOM_BULLET_DESCRIPTION));
		String aReplace = String::CreateFromAscii("%LIST_NUM");
		String sNUM = String::CreateFromInt32( nIndex + 1 );
		aStrFromRES.SearchAndReplace(aReplace,sNUM);
		pEntry->sDescription = aStrFromRES;
	}else
	{
		return sal_False;
	}

	return sal_True;
}

sal_Bool GraphicBulletsTypeMgr::ApplyNumRule(SvxNumRule& aNum,sal_uInt16 nIndex,sal_uInt16 mLevel,sal_Bool /* isDefault */,sal_Bool /* isResetSize */)
{
	//if ( mLevel == (sal_uInt16)0xFFFF )
	//	return sal_False;

	if ( nIndex >= aGrfDataLst.Count() )
		return sal_False;

	String sGrfName;
	GrfBulDataRelation* pEntry = (GrfBulDataRelation*) aGrfDataLst.GetObject(nIndex);
	sGrfName= pEntry->sGrfName;

	sal_uInt16 nMask = 1;
	String aEmptyStr;
	sal_uInt16 nSetNumberingType = SVX_NUM_BITMAP;
	String sNumCharFmtName = GetBulCharFmtName();
	for(sal_uInt16 i = 0; i < aNum.GetLevelCount(); i++)
	{
		if(mLevel & nMask)
		{
			SvxNumberFormat aFmt(aNum.GetLevel(i));
			aFmt.SetNumberingType(nSetNumberingType);
			aFmt.SetPrefix( aEmptyStr );
			aFmt.SetSuffix( aEmptyStr );
			aFmt.SetCharFmtName( sNumCharFmtName );

			Graphic aGraphic;
			if(GalleryExplorer::GetGraphicObj( GALLERY_THEME_BULLETS, pEntry->nGallaryIndex, &aGraphic))
			{
				Size aSize = SvxNumberFormat::GetGraphicSizeMM100(&aGraphic);
				sal_Int16 eOrient = text::VertOrientation::LINE_CENTER;
				aSize = OutputDevice::LogicToLogic(aSize, MAP_100TH_MM, (MapUnit)GetMapUnit());
				SvxBrushItem aBrush(aGraphic, GPOS_AREA, SID_ATTR_BRUSH );
				aFmt.SetGraphicBrush( &aBrush, &aSize, &eOrient );
			}
			else // if(pGrfName)
				aFmt.SetGraphic( sGrfName );

			aNum.SetLevel(i, aFmt);
		}
		nMask <<= 1 ;
	}

	return sal_True;
}
String GraphicBulletsTypeMgr::GetDescription(sal_uInt16 nIndex,sal_Bool /* isDefault */)
{
	String sRet;
	sal_uInt16 nLength = 0;
	nLength = aGrfDataLst.Count() ;

	if ( nIndex >= nLength )
		return sRet;
	else
	{
		GrfBulDataRelation* pEntry = (GrfBulDataRelation*) aGrfDataLst.GetObject(nIndex);
		if ( pEntry )
		{
			sRet = pEntry->sDescription;
		};
	}
	return sRet;
}
sal_Bool GraphicBulletsTypeMgr::IsCustomized(sal_uInt16 nIndex)
{
	sal_Bool bRet = sal_False;

	sal_uInt16 nLength = 0;
	nLength = aGrfDataLst.Count() ;

	if ( nIndex >= nLength )
		return bRet;
	else
	{
		GrfBulDataRelation* pEntry = (GrfBulDataRelation*) aGrfDataLst.GetObject(nIndex);
		if ( pEntry )
		{
			bRet = pEntry->bIsCustomized;
		};
	}

	return bRet;
}
String GraphicBulletsTypeMgr::GetGrfName(sal_uInt16 nIndex)
{
	String sRet;
	if ( nIndex < aGrfDataLst.Count() )
	{
		GrfBulDataRelation* pEntry = (GrfBulDataRelation*) aGrfDataLst.GetObject(nIndex);
		if ( pEntry )
		{
			sRet = pEntry->sGrfName;
		}
	}

	return sRet;
}
/***************************************************************************************************
**********************Mix Bullets Type lib**************************************************************
****************************************************************************************************/
MixBulletsTypeMgr* MixBulletsTypeMgr::_instance = 0;
MixBulletsSettings_Impl* MixBulletsTypeMgr::pActualBullets[] ={0,0,0,0,0,0,0,0};
MixBulletsSettings_Impl* MixBulletsTypeMgr::pDefaultActualBullets[] ={0,0,0,0,0,0,0,0};

MixBulletsTypeMgr::MixBulletsTypeMgr(const NBOType aType):
	NBOTypeMgrBase(aType)
{
	Init();
	for(sal_Int32 nItem = 0; nItem < DEFAULT_BULLET_TYPES; nItem++ )
	{
		pDefaultActualBullets[nItem] = pActualBullets[nItem];
	}
	// Initialize the first time to store the default value. Then do it again for customized value
	Init();
	ImplLoad(String::CreateFromAscii("standard.sya"));
}

MixBulletsTypeMgr::MixBulletsTypeMgr(const NBOType aType,const SfxItemSet* pArg):
	NBOTypeMgrBase(aType,pArg)
{
	Init();
	for(sal_Int32 nItem = 0; nItem < DEFAULT_BULLET_TYPES; nItem++ )
	{
		pDefaultActualBullets[nItem] = pActualBullets[nItem];
	}
	// Initialize the first time to store the default value. Then do it again for customized value
	Init();
	ImplLoad(String::CreateFromAscii("standard.sya"));
}

MixBulletsTypeMgr::MixBulletsTypeMgr(const MixBulletsTypeMgr& aTypeMgr):
	NBOTypeMgrBase(aTypeMgr)
{
	for (sal_uInt16 i=0;i<DEFAULT_BULLET_TYPES;i++)
	{
		if ( aTypeMgr.pActualBullets[i]->eType == eNBType::BULLETS )
		{
			pActualBullets[i]->eType = aTypeMgr.pActualBullets[i]->eType;
			pActualBullets[i]->nIndex = aTypeMgr.pActualBullets[i]->nIndex; // Index in the tab page display
			pActualBullets[i]->nIndexDefault = aTypeMgr.pActualBullets[i]->nIndexDefault;
			pActualBullets[i]->pBullets = new BulletsSettings_Impl(eNBType::BULLETS) ;
			((BulletsSettings_Impl*)(pActualBullets[i]->pBullets))->cBulletChar = ((BulletsSettings_Impl*)(aTypeMgr.pActualBullets[i]->pBullets))->cBulletChar;
			((BulletsSettings_Impl*)(pActualBullets[i]->pBullets))->aFont = ((BulletsSettings_Impl*)(aTypeMgr.pActualBullets[i]->pBullets))->aFont;
			((BulletsSettings_Impl*)(pActualBullets[i]->pBullets))->sDescription = ((BulletsSettings_Impl*)(aTypeMgr.pActualBullets[i]->pBullets))->sDescription;
			((BulletsSettings_Impl*)(pActualBullets[i]->pBullets))->bIsCustomized = ((BulletsSettings_Impl*)(aTypeMgr.pActualBullets[i]->pBullets))->bIsCustomized;
			((BulletsSettings_Impl*)(pActualBullets[i]->pBullets))->eType = ((BulletsSettings_Impl*)(aTypeMgr.pActualBullets[i]->pBullets))->eType;
		}else if ( aTypeMgr.pActualBullets[i]->eType == eNBType::GRAPHICBULLETS )
		{
			pActualBullets[i]->eType = aTypeMgr.pActualBullets[i]->eType;
			pActualBullets[i]->nIndex = aTypeMgr.pActualBullets[i]->nIndex; // Index in the tab page display
			pActualBullets[i]->nIndexDefault = aTypeMgr.pActualBullets[i]->nIndexDefault;
			pActualBullets[i]->pBullets = new GrfBulDataRelation(eNBType::GRAPHICBULLETS) ;
			((GrfBulDataRelation*)(pActualBullets[i]->pBullets))->sGrfName = ((GrfBulDataRelation*)(aTypeMgr.pActualBullets[i]->pBullets))->sGrfName;
			((GrfBulDataRelation*)(pActualBullets[i]->pBullets))->sDescription = ((GrfBulDataRelation*)(aTypeMgr.pActualBullets[i]->pBullets))->sDescription;
			((GrfBulDataRelation*)(pActualBullets[i]->pBullets))->bIsCustomized = ((GrfBulDataRelation*)(aTypeMgr.pActualBullets[i]->pBullets))->bIsCustomized;
			((GrfBulDataRelation*)(pActualBullets[i]->pBullets))->eType = ((GrfBulDataRelation*)(aTypeMgr.pActualBullets[i]->pBullets))->eType;
			if ( ((GrfBulDataRelation*)(aTypeMgr.pActualBullets[i]->pBullets))->bIsCustomized && ((GrfBulDataRelation*)(aTypeMgr.pActualBullets[i]->pBullets))->pGrfObj != NULL)
			{
				((GrfBulDataRelation*)(pActualBullets[i]->pBullets))->pGrfObj = ((GrfBulDataRelation*)(aTypeMgr.pActualBullets[i]->pBullets))->pGrfObj;
			}
		}
	}
	ImplLoad(String::CreateFromAscii("standard.sya"));
}
void MixBulletsTypeMgr::Init()
{
	BulletsTypeMgr* pBTMgr = BulletsTypeMgr::GetInstance();
	if ( pBTMgr )
	{
		// Index 1
		pActualBullets[0] = new MixBulletsSettings_Impl(eNBType::BULLETS);
		pActualBullets[0]->eType = eNBType::BULLETS;
		pActualBullets[0]->nIndex = 0+1; // Index in the tab page display,decrease 1 to the index within arr
		pActualBullets[0]->nIndexDefault = 2; // Index in the tab page display,decrease 1 to the index within arr
		pActualBullets[0]->pBullets = new BulletsSettings_Impl(eNBType::BULLETS) ;
		((BulletsSettings_Impl*)(pActualBullets[0]->pBullets))->cBulletChar = pBTMgr->GetBulChar(pActualBullets[0]->nIndexDefault-1);
		((BulletsSettings_Impl*)(pActualBullets[0]->pBullets))->aFont = pBTMgr->GetBulCharFont(pActualBullets[0]->nIndexDefault-1);
		((BulletsSettings_Impl*)(pActualBullets[0]->pBullets))->sDescription = pBTMgr->GetDescription(pActualBullets[0]->nIndexDefault-1);
		((BulletsSettings_Impl*)(pActualBullets[0]->pBullets))->bIsCustomized = pBTMgr->IsCustomized(pActualBullets[0]->nIndexDefault-1);
		((BulletsSettings_Impl*)(pActualBullets[0]->pBullets))->eType = eNBType::BULLETS;

		// Index 2
		pActualBullets[1] = new MixBulletsSettings_Impl(eNBType::BULLETS);
		pActualBullets[1]->eType = eNBType::BULLETS;
		pActualBullets[1]->nIndex = 1+1; // Index in the tab page display,decrease 1 to the index within arr
		pActualBullets[1]->nIndexDefault = 3; // Index in the tab page display,decrease 1 to the index within arr
		pActualBullets[1]->pBullets = new BulletsSettings_Impl(eNBType::BULLETS) ;
		((BulletsSettings_Impl*)(pActualBullets[1]->pBullets))->cBulletChar = pBTMgr->GetBulChar(pActualBullets[1]->nIndexDefault-1);
		((BulletsSettings_Impl*)(pActualBullets[1]->pBullets))->aFont = pBTMgr->GetBulCharFont(pActualBullets[1]->nIndexDefault-1);
		((BulletsSettings_Impl*)(pActualBullets[1]->pBullets))->sDescription = pBTMgr->GetDescription(pActualBullets[1]->nIndexDefault-1);
		((BulletsSettings_Impl*)(pActualBullets[1]->pBullets))->bIsCustomized = pBTMgr->IsCustomized(pActualBullets[1]->nIndexDefault-1);
		((BulletsSettings_Impl*)(pActualBullets[1]->pBullets))->eType = eNBType::BULLETS;

		// Index 3
		pActualBullets[2] = new MixBulletsSettings_Impl(eNBType::BULLETS);
		pActualBullets[2]->eType = eNBType::BULLETS;
		pActualBullets[2]->nIndex = 2+1; // Index in the tab page display,decrease 1 to the index within arr
		pActualBullets[2]->nIndexDefault = 4; // Index in the tab page display,decrease 1 to the index within arr
		pActualBullets[2]->pBullets = new BulletsSettings_Impl(eNBType::BULLETS) ;
		((BulletsSettings_Impl*)(pActualBullets[2]->pBullets))->cBulletChar = pBTMgr->GetBulChar(pActualBullets[2]->nIndexDefault-1);
		((BulletsSettings_Impl*)(pActualBullets[2]->pBullets))->aFont = pBTMgr->GetBulCharFont(pActualBullets[2]->nIndexDefault-1);
		((BulletsSettings_Impl*)(pActualBullets[2]->pBullets))->sDescription = pBTMgr->GetDescription(pActualBullets[2]->nIndexDefault-1);
		((BulletsSettings_Impl*)(pActualBullets[2]->pBullets))->bIsCustomized = pBTMgr->IsCustomized(pActualBullets[2]->nIndexDefault-1);
		((BulletsSettings_Impl*)(pActualBullets[2]->pBullets))->eType = eNBType::BULLETS;

		// Index 4
		pActualBullets[3] = new MixBulletsSettings_Impl(eNBType::BULLETS);
		pActualBullets[3]->eType = eNBType::BULLETS;
		pActualBullets[3]->nIndex = 3+1; // Index in the tab page display,decrease 1 to the index within arr
		pActualBullets[3]->nIndexDefault = 5; // Index in the tab page display,decrease 1 to the index within arr
		pActualBullets[3]->pBullets = new BulletsSettings_Impl(eNBType::BULLETS) ;
		((BulletsSettings_Impl*)(pActualBullets[3]->pBullets))->cBulletChar = pBTMgr->GetBulChar(pActualBullets[3]->nIndexDefault-1);
		((BulletsSettings_Impl*)(pActualBullets[3]->pBullets))->aFont = pBTMgr->GetBulCharFont(pActualBullets[3]->nIndexDefault-1);
		((BulletsSettings_Impl*)(pActualBullets[3]->pBullets))->sDescription = pBTMgr->GetDescription(pActualBullets[3]->nIndexDefault-1);
		((BulletsSettings_Impl*)(pActualBullets[3]->pBullets))->bIsCustomized = pBTMgr->IsCustomized(pActualBullets[3]->nIndexDefault-1);
		((BulletsSettings_Impl*)(pActualBullets[3]->pBullets))->eType = eNBType::BULLETS;

		// Index 5
		pActualBullets[4] = new MixBulletsSettings_Impl(eNBType::BULLETS);
		pActualBullets[4]->eType = eNBType::BULLETS;
		pActualBullets[4]->nIndex = 4+1; // Index in the tab page display,decrease 1 to the index within arr
		pActualBullets[4]->nIndexDefault = 6; // Index in the tab page display,decrease 1 to the index within arr
		pActualBullets[4]->pBullets = new BulletsSettings_Impl(eNBType::BULLETS) ;
		((BulletsSettings_Impl*)(pActualBullets[4]->pBullets))->cBulletChar = pBTMgr->GetBulChar(pActualBullets[4]->nIndexDefault-1);
		((BulletsSettings_Impl*)(pActualBullets[4]->pBullets))->aFont = pBTMgr->GetBulCharFont(pActualBullets[4]->nIndexDefault-1);
		((BulletsSettings_Impl*)(pActualBullets[4]->pBullets))->sDescription = pBTMgr->GetDescription(pActualBullets[4]->nIndexDefault-1);
		((BulletsSettings_Impl*)(pActualBullets[4]->pBullets))->bIsCustomized = pBTMgr->IsCustomized(pActualBullets[4]->nIndexDefault-1);
		((BulletsSettings_Impl*)(pActualBullets[4]->pBullets))->eType = eNBType::BULLETS;

		// Index 6
		pActualBullets[5] = new MixBulletsSettings_Impl(eNBType::BULLETS);
		pActualBullets[5]->eType = eNBType::BULLETS;
		pActualBullets[5]->nIndex = 5+1; // Index in the tab page display,decrease 1 to the index within arr
		pActualBullets[5]->nIndexDefault = 8; // Index in the tab page display,decrease 1 to the index within arr
		pActualBullets[5]->pBullets = new BulletsSettings_Impl(eNBType::BULLETS) ;
		((BulletsSettings_Impl*)(pActualBullets[5]->pBullets))->cBulletChar = pBTMgr->GetBulChar(pActualBullets[5]->nIndexDefault-1);
		((BulletsSettings_Impl*)(pActualBullets[5]->pBullets))->aFont = pBTMgr->GetBulCharFont(pActualBullets[5]->nIndexDefault-1);
		((BulletsSettings_Impl*)(pActualBullets[5]->pBullets))->sDescription = pBTMgr->GetDescription(pActualBullets[5]->nIndexDefault-1);
		((BulletsSettings_Impl*)(pActualBullets[5]->pBullets))->bIsCustomized = pBTMgr->IsCustomized(pActualBullets[5]->nIndexDefault-1);
		((BulletsSettings_Impl*)(pActualBullets[5]->pBullets))->eType = eNBType::BULLETS;
	}

	GraphicBulletsTypeMgr* mGrfTMgr = GraphicBulletsTypeMgr::GetInstance();
	if ( mGrfTMgr )
	{
		// Index 7
		pActualBullets[6] = new MixBulletsSettings_Impl(eNBType::GRAPHICBULLETS);
		pActualBullets[6]->eType = eNBType::GRAPHICBULLETS;
		pActualBullets[6]->nIndex = 6+1; // Index in the tab page display,decrease 1 to the index within arr
		pActualBullets[6]->nIndexDefault = 9; // Index in the tab page display,decrease 1 to the index within arr
		pActualBullets[6]->pBullets = new GrfBulDataRelation(eNBType::GRAPHICBULLETS) ;
		((GrfBulDataRelation*)(pActualBullets[6]->pBullets))->sGrfName = mGrfTMgr->GetGrfName(pActualBullets[6]->nIndexDefault);
		((GrfBulDataRelation*)(pActualBullets[6]->pBullets))->sDescription = mGrfTMgr->GetDescription(pActualBullets[6]->nIndexDefault);
		((GrfBulDataRelation*)(pActualBullets[6]->pBullets))->bIsCustomized = mGrfTMgr->IsCustomized(pActualBullets[6]->nIndexDefault);
		((GrfBulDataRelation*)(pActualBullets[6]->pBullets))->eType = eNBType::GRAPHICBULLETS;

		// Index 8
		pActualBullets[7] = new MixBulletsSettings_Impl(eNBType::GRAPHICBULLETS);
		pActualBullets[7]->eType = eNBType::GRAPHICBULLETS;
		pActualBullets[7]->nIndex = 7+1; // Index in the tab page display,decrease 1 to the index within arr
		pActualBullets[7]->nIndexDefault = 23; // Index in the tab page display,decrease 1 to the index within arr
		pActualBullets[7]->pBullets = new GrfBulDataRelation(eNBType::GRAPHICBULLETS) ;
		((GrfBulDataRelation*)(pActualBullets[7]->pBullets))->sGrfName = mGrfTMgr->GetGrfName(pActualBullets[7]->nIndexDefault);
		((GrfBulDataRelation*)(pActualBullets[7]->pBullets))->sDescription = mGrfTMgr->GetDescription(pActualBullets[7]->nIndexDefault);
		((GrfBulDataRelation*)(pActualBullets[7]->pBullets))->bIsCustomized = mGrfTMgr->IsCustomized(pActualBullets[7]->nIndexDefault);
		((GrfBulDataRelation*)(pActualBullets[7]->pBullets))->eType = eNBType::GRAPHICBULLETS;
	}

}
sal_uInt16 MixBulletsTypeMgr::GetNBOIndexForNumRule(SvxNumRule& aNum,sal_uInt16 mLevel,sal_uInt16 nFromIndex)
{
	if ( mLevel == (sal_uInt16)0xFFFF || mLevel == 0)
		return (sal_uInt16)0xFFFF;
	//if ( !lcl_IsNumFmtSet(pNR, mLevel) ) return (sal_uInt16)0xFFFF;

	sal_uInt16 nActLv = IsSingleLevel(mLevel);

	if ( nActLv == (sal_uInt16)0xFFFF )
		return (sal_uInt16)0xFFFF;

	SvxNumberFormat aFmt(aNum.GetLevel(nActLv));
	sal_Int16 eNumType = aFmt.GetNumberingType();
	if( eNumType == SVX_NUM_CHAR_SPECIAL)
	{
		sal_Unicode cChar = aFmt.GetBulletChar();
		const Font* pFont = aFmt.GetBulletFont();
		String sName = pFont?pFont->GetName():String();

		for(sal_uInt16 i = nFromIndex; i < DEFAULT_BULLET_TYPES; i++)
		{
			if ( pActualBullets[i]->eType == eNBType::BULLETS )
			{
				String ssName = ((BulletsSettings_Impl*)(pActualBullets[i]->pBullets))->aFont.GetName();
				if ( (cChar == ((BulletsSettings_Impl*)(pActualBullets[i]->pBullets))->cBulletChar||
					(cChar == 9830 && 57356 == ((BulletsSettings_Impl*)(pActualBullets[i]->pBullets))->cBulletChar) ||
					(cChar == 9632 && 57354 == ((BulletsSettings_Impl*)(pActualBullets[i]->pBullets))->cBulletChar)))//&&
					//(pFont && pFont->GetName().CompareTo(((BulletsSettings_Impl*)(pActualBullets[i]->pBullets))->aFont.GetName())==COMPARE_EQUAL) )
				{
					return pActualBullets[i]->nIndex;
				}
			}
		}
	}else if ( (eNumType&(~LINK_TOKEN)) == SVX_NUM_BITMAP )
	{
		const SvxBrushItem* pBrsh = aFmt.GetBrush();
		const Graphic* pGrf = 0;
		if ( pBrsh )
			pGrf = pBrsh->GetGraphic();

		if ( pGrf )
		{
			for(sal_uInt16 i = nFromIndex; i < DEFAULT_BULLET_TYPES; i++)
			{
				if ( pActualBullets[i]->eType == eNBType::GRAPHICBULLETS )
				{
					GrfBulDataRelation* pEntry = (GrfBulDataRelation*) (pActualBullets[i]->pBullets);
					//sal_Bool bExist = sal_False;
					if ( pEntry && pActualBullets[i]->nIndexDefault == (sal_uInt16)0xFFFF && pEntry->pGrfObj)
					{
						if ( pEntry->pGrfObj->GetBitmap().IsEqual(pGrf->GetBitmap()))
						{
							return pActualBullets[i]->nIndex;
						}
					}else { //if ( pEntry && pGrfName && pEntry->sGrfName.CompareTo(*pGrfName)==COMPARE_EQUAL )
						//bExist = GalleryExplorer::GetGraphicObj(GALLERY_THEME_BULLETS, pActualBullets[i]->nIndexDefault-1,pSrGrf);
						Graphic aSrGrf;
						if (pEntry)
							GalleryExplorer::GetGraphicObj(GALLERY_THEME_BULLETS, pActualBullets[i]->nIndexDefault,&aSrGrf);
						Bitmap aSum=pGrf->GetBitmap();
						Bitmap aSum1=aSrGrf.GetBitmap();
						if (aSum.IsEqual(aSum1))
							return pActualBullets[i]->nIndex;
					}
				}
			}
		}
	}

	return (sal_uInt16)0xFFFF;
}

sal_Bool MixBulletsTypeMgr::RelplaceNumRule(SvxNumRule& aNum,sal_uInt16 nIndex,sal_uInt16 mLevel)
{
	if ( mLevel == (sal_uInt16)0xFFFF || mLevel == 0 || nIndex>=DEFAULT_BULLET_TYPES)
		return sal_False;

	//if ( GetNBOIndexForNumRule(aNum,mLevel) != (sal_uInt16)0xFFFF )
	//	return sal_False;

	sal_uInt16 nActLv = IsSingleLevel(mLevel);

	if ( nActLv == (sal_uInt16)0xFFFF )
		return sal_False;

	SvxNumberFormat aFmt(aNum.GetLevel(nActLv));
	sal_Int16 eNumType = aFmt.GetNumberingType();
	if( eNumType == SVX_NUM_CHAR_SPECIAL && pActualBullets[nIndex]->eType == eNBType::BULLETS )
	{
		sal_Unicode cChar = aFmt.GetBulletChar();
		const Font* pFont = aFmt.GetBulletFont();
		BulletsSettings_Impl* pEntry = (BulletsSettings_Impl*) (pActualBullets[nIndex]->pBullets);
		pEntry->cBulletChar = cChar;
		pEntry->aFont = pFont?*pFont:lcl_GetDefaultBulletFont();
		pEntry->bIsCustomized = sal_True;
		String aStrFromRES = String(SVX_RESSTR( RID_SVXSTR_NUMBULLET_CUSTOM_BULLET_DESCRIPTION));
		String aReplace = String::CreateFromAscii("%LIST_NUM");
		String sNUM = String::CreateFromInt32( nIndex + 1 );
		aStrFromRES.SearchAndReplace(aReplace,sNUM);
		pEntry->sDescription = aStrFromRES;

	}else if ( (eNumType&(~LINK_TOKEN)) == SVX_NUM_BITMAP && pActualBullets[nIndex]->eType == eNBType::GRAPHICBULLETS )
	{
		const SvxBrushItem* pBrsh = aFmt.GetBrush();
		const Graphic* pGrf = 0;
		if ( pBrsh )
			pGrf = pBrsh->GetGraphic();
		else
			return sal_False;

		String sEmpty;
		if ( pGrf )
		{
			const String* pGrfName = pBrsh->GetGraphicLink();
			//String* pGrfName = (String*)(pBrsh->GetGraphicLink());
			GrfBulDataRelation* pEntry = (GrfBulDataRelation*) (pActualBullets[nIndex]->pBullets);
			if ( pGrfName )
				pEntry->sGrfName = *pGrfName;
			GraphicBulletsTypeMgr* mGrfTMgr = GraphicBulletsTypeMgr::GetInstance();
			if ( mGrfTMgr )
			{
				//sal_uInt16 nDIndex = mGrfTMgr->GetNBOIndexForNumRule(aNum,mLevel);
				//if ( nDIndex != (sal_uInt16)0xFFFF)
				//{
				//	pActualBullets[nIndex]->nIndexDefault = nDIndex -1;
				//	sEmpty = mGrfTMgr->GetDescription( nDIndex -1);
				//}else
				{
					pActualBullets[nIndex]->nIndexDefault = (sal_uInt16)0xFFFF;
					sEmpty = String(SVX_RESSTR( RID_SVXSTR_NUMBULLET_CUSTOM_BULLET_DESCRIPTION));
					String aReplace = String::CreateFromAscii("%LIST_NUM");
					String sNUM = String::CreateFromInt32( nIndex + 1 );
					sEmpty.SearchAndReplace(aReplace,sNUM);
					//pEntry->pGrfObj = pGrf;
					pEntry->pGrfObj = new Graphic(*pGrf);
					pEntry->aSize = aFmt.GetGraphicSize();
					pEntry->aSize = OutputDevice::LogicToLogic(pEntry->aSize,(MapUnit)GetMapUnit(),MAP_100TH_MM);
					sal_uInt16 nDIndex = mGrfTMgr->GetNBOIndexForNumRule(aNum,mLevel);
					if (nDIndex!=(sal_uInt16)0xFFFF) pEntry->aSize=Size(0,0);
				}
			}
			pEntry->sDescription = sEmpty;
			pEntry->bIsCustomized = sal_True;
		}else
		{
			return sal_False;
		}
	}else
	{
		delete pActualBullets[nIndex]->pBullets;
		pActualBullets[nIndex]->pBullets = 0;
		if ( eNumType == SVX_NUM_CHAR_SPECIAL )
		{
			sal_Unicode cChar = aFmt.GetBulletChar();
			const Font* pFont = aFmt.GetBulletFont();
			pActualBullets[nIndex]->eType = eNBType::BULLETS;
			pActualBullets[nIndex]->nIndex = nIndex+1; // Index in the tab page display, decrease 1 to the index within arr
			pActualBullets[nIndex]->pBullets = new BulletsSettings_Impl(eNBType::BULLETS) ;
			((BulletsSettings_Impl*)(pActualBullets[nIndex]->pBullets))->cBulletChar = cChar;
			((BulletsSettings_Impl*)(pActualBullets[nIndex]->pBullets))->aFont = pFont?*pFont:lcl_GetDefaultBulletFont();
			((BulletsSettings_Impl*)(pActualBullets[nIndex]->pBullets))->bIsCustomized = sal_True;
			((BulletsSettings_Impl*)(pActualBullets[nIndex]->pBullets))->eType = eNBType::BULLETS;
			BulletsTypeMgr* pBTMgr = BulletsTypeMgr::GetInstance();
			if ( pBTMgr )
			{
				//sal_uInt16 nDIndex = pBTMgr->GetNBOIndexForNumRule(aNum,mLevel);
				//if ( nDIndex != (sal_uInt16)0xFFFF)
				//{
				//	pActualBullets[nIndex]->nIndexDefault = nDIndex -1;
				//	((BulletsSettings_Impl*)(pActualBullets[nIndex]->pBullets))->sDescription = pBTMgr->GetDescription(nDIndex - 1);
				//}else
				{
					pActualBullets[nIndex]->nIndexDefault = (sal_uInt16)0xFFFF;
					String aStrFromRES = String(SVX_RESSTR( RID_SVXSTR_NUMBULLET_CUSTOM_BULLET_DESCRIPTION));
					String aReplace = String::CreateFromAscii("%LIST_NUM");
					String sNUM = String::CreateFromInt32( nIndex + 1 );
					aStrFromRES.SearchAndReplace(aReplace,sNUM);
					((BulletsSettings_Impl*)(pActualBullets[nIndex]->pBullets))->sDescription = aStrFromRES;
				}
			}
		}else if ( (eNumType&(~LINK_TOKEN)) == SVX_NUM_BITMAP )
		{
			const SvxBrushItem* pBrsh = aFmt.GetBrush();
			const Graphic* pGrf = 0;
			if ( pBrsh )
				pGrf = pBrsh->GetGraphic();
			else
				return sal_False;

			String sEmpty;
			const String* pGrfName = 0;
			if ( pGrf )
			{
				pGrfName = pBrsh->GetGraphicLink();

				pActualBullets[nIndex]->eType = eNBType::GRAPHICBULLETS;
				pActualBullets[nIndex]->nIndex = nIndex+1; // Index in the tab page display, decrease 1 to the index within arr
				pActualBullets[nIndex]->pBullets = new GrfBulDataRelation(eNBType::GRAPHICBULLETS) ;
				if (pGrfName)
					((GrfBulDataRelation*)(pActualBullets[nIndex]->pBullets))->sGrfName = *pGrfName;
				((GrfBulDataRelation*)(pActualBullets[nIndex]->pBullets))->bIsCustomized = sal_True;
				((GrfBulDataRelation*)(pActualBullets[nIndex]->pBullets))->eType = eNBType::GRAPHICBULLETS;
				GraphicBulletsTypeMgr* mGrfTMgr = GraphicBulletsTypeMgr::GetInstance();
				if ( mGrfTMgr )
				{
					//sal_uInt16 nDIndex = mGrfTMgr->GetNBOIndexForNumRule(aNum,mLevel);
					//if ( nDIndex != (sal_uInt16)0xFFFF)
					//{
					//	pActualBullets[nIndex]->nIndexDefault = nDIndex - 1;
					//	((GrfBulDataRelation*)(pActualBullets[nIndex]->pBullets))->sDescription = mGrfTMgr->GetDescription(nDIndex - 1);
					//}else
					{
						pActualBullets[nIndex]->nIndexDefault = (sal_uInt16)0xFFFF;
						String aStrFromRES = String(SVX_RESSTR( RID_SVXSTR_NUMBULLET_CUSTOM_BULLET_DESCRIPTION));
						String aReplace = String::CreateFromAscii("%LIST_NUM");
						String sNUM = String::CreateFromInt32( nIndex + 1 );
						aStrFromRES.SearchAndReplace(aReplace,sNUM);
						((GrfBulDataRelation*)(pActualBullets[nIndex]->pBullets))->sDescription = aStrFromRES;
						//((GrfBulDataRelation*)(pActualBullets[nIndex]->pBullets))->pGrfObj = pGrf;
						((GrfBulDataRelation*)(pActualBullets[nIndex]->pBullets))->pGrfObj = new Graphic(*pGrf);
						((GrfBulDataRelation*)(pActualBullets[nIndex]->pBullets))->pGrfObj = new Graphic(*pGrf);
						Size aTmpSize = aFmt.GetGraphicSize();
						aTmpSize = OutputDevice::LogicToLogic(aTmpSize,(MapUnit)GetMapUnit(),MAP_100TH_MM);
						sal_uInt16 nDIndex = mGrfTMgr->GetNBOIndexForNumRule(aNum,mLevel);
						if (nDIndex!=(sal_uInt16)0xFFFF) aTmpSize=Size(0,0);
						((GrfBulDataRelation*)(pActualBullets[nIndex]->pBullets))->aSize = aTmpSize;

				}
				}
			}
		}
	}
	SvxNumRule aTmpRule1(aNum);
	ApplyNumRule(aTmpRule1,nIndex,mLevel,true);
	if (GetNBOIndexForNumRule(aTmpRule1,mLevel,nIndex)==nIndex+1) {
		if (pActualBullets[nIndex]->eType == eNBType::BULLETS) {
			BulletsSettings_Impl* pEntry = (BulletsSettings_Impl*) (pActualBullets[nIndex]->pBullets);
			pEntry->bIsCustomized = false;
			pEntry->sDescription = GetDescription(nIndex,true);
		}
		if (pActualBullets[nIndex]->eType == eNBType::GRAPHICBULLETS) {
			GrfBulDataRelation* pEntry = (GrfBulDataRelation*) (pActualBullets[nIndex]->pBullets);
			pEntry->bIsCustomized = false;
			pEntry->sDescription = GetDescription(nIndex,true);
		}
	}
	ImplStore(String::CreateFromAscii("standard.sya"));
	return sal_True;
}

sal_Bool MixBulletsTypeMgr::ApplyNumRule(SvxNumRule& aNum,sal_uInt16 nIndex,sal_uInt16 mLevel,sal_Bool isDefault,sal_Bool isResetSize)
{
	//if ( mLevel == (sal_uInt16)0xFFFF || nIndex>=DEFAULT_BULLET_TYPES )
	if ( nIndex>=DEFAULT_BULLET_TYPES )
		return sal_False;
	MixBulletsSettings_Impl* pCurrentBullets = pActualBullets[nIndex];
	if (isDefault) pCurrentBullets=pDefaultActualBullets[nIndex];

	if ( pCurrentBullets->eType == eNBType::BULLETS )
	{
		sal_Unicode cChar;
		cChar = ((BulletsSettings_Impl*)(pCurrentBullets->pBullets))->cBulletChar;

		//Font& rActBulletFont = lcl_GetDefaultBulletFont();
		Font rActBulletFont = ((BulletsSettings_Impl*)(pCurrentBullets->pBullets))->aFont;
		sal_uInt16 nMask = 1;
		String sBulletCharFmtName = GetBulCharFmtName();
		for(sal_uInt16 i = 0; i < aNum.GetLevelCount(); i++)
		{
			if(mLevel & nMask)
			{
				SvxNumberFormat aFmt(aNum.GetLevel(i));
				if (SVX_NUM_CHAR_SPECIAL !=aFmt.GetNumberingType()) isResetSize=true;
				aFmt.SetNumberingType( SVX_NUM_CHAR_SPECIAL );
				aFmt.SetBulletFont(&rActBulletFont);
				aFmt.SetBulletChar(cChar );
				aFmt.SetCharFmtName(sBulletCharFmtName);
				String aEmptyStr;
				aFmt.SetPrefix( aEmptyStr );
				aFmt.SetSuffix( aEmptyStr );
				if (isResetSize) aFmt.SetBulletRelSize(45);
				aNum.SetLevel(i, aFmt);
			}
			nMask <<= 1;
		}
	}else if ( pCurrentBullets->eType == eNBType::GRAPHICBULLETS )
	{
		String sGrfName;
		GrfBulDataRelation* pEntry = (GrfBulDataRelation*) (pCurrentBullets->pBullets);
		sGrfName= pEntry->sGrfName;

		sal_uInt16 nMask = 1;
		String aEmptyStr;
		sal_uInt16 nSetNumberingType = SVX_NUM_BITMAP;
		String sNumCharFmtName = GetBulCharFmtName();
		for(sal_uInt16 i = 0; i < aNum.GetLevelCount(); i++)
		{
			if(mLevel & nMask)
			{
				SvxNumberFormat aFmt(aNum.GetLevel(i));
				if (SVX_NUM_BITMAP !=aFmt.GetNumberingType()) isResetSize=true;
				aFmt.SetNumberingType(nSetNumberingType);
				aFmt.SetPrefix( aEmptyStr );
				aFmt.SetSuffix( aEmptyStr );
				aFmt.SetCharFmtName( sNumCharFmtName );
				if ( pCurrentBullets->nIndexDefault == (sal_uInt16)0xFFFF && pEntry->pGrfObj )
				{
					Size aSize = pEntry->aSize;
					sal_Int16 eOrient = text::VertOrientation::LINE_CENTER;
					if (!isResetSize && aFmt.GetGraphicSize()!=Size(0,0)) aSize=aFmt.GetGraphicSize();
					else {
						if (aSize.Width()==0 && aSize.Height()==0) {
							aSize = SvxNumberFormat::GetGraphicSizeMM100( pEntry->pGrfObj );
						}
						aSize = OutputDevice::LogicToLogic(aSize, MAP_100TH_MM, (MapUnit)GetMapUnit());
					}
					SvxBrushItem aBrush(*(pEntry->pGrfObj), GPOS_AREA, SID_ATTR_BRUSH );
					aFmt.SetGraphicBrush( &aBrush, &aSize, &eOrient );
				}else
				{
					Graphic aGraphic;
					if(GalleryExplorer::GetGraphicObj( GALLERY_THEME_BULLETS, pCurrentBullets->nIndexDefault, &aGraphic))
					{
								Size aSize = pEntry->aSize;
								sal_Int16 eOrient = text::VertOrientation::LINE_CENTER;
								if (!isResetSize && aFmt.GetGraphicSize()!=Size(0,0)) aSize=aFmt.GetGraphicSize();
								else {
									if (aSize.Width()==0 && aSize.Height()==0) {
										aSize = SvxNumberFormat::GetGraphicSizeMM100(&aGraphic);
									}
									aSize = OutputDevice::LogicToLogic(aSize, MAP_100TH_MM, (MapUnit)GetMapUnit());
								}
								SvxBrushItem aBrush(aGraphic, GPOS_AREA, SID_ATTR_BRUSH );
								aFmt.SetGraphicBrush( &aBrush, &aSize, &eOrient );
					}else
						aFmt.SetGraphic( sGrfName );
				}

				aNum.SetLevel(i, aFmt);
			}
			nMask <<= 1 ;
		}
	}

	return sal_True;
}

String MixBulletsTypeMgr::GetDescription(sal_uInt16 nIndex,sal_Bool isDefault)
{
	String sRet;
	//sal_uInt16 nLength = 0;
	//nLength = sizeof(pActualBullets)/sizeof(BulletsSettings_Impl);

	if ( nIndex >= DEFAULT_BULLET_TYPES )
		return sRet;
	else
		sRet = pActualBullets[nIndex]->pBullets->sDescription;
	if (isDefault) sRet = pDefaultActualBullets[nIndex]->pBullets->sDescription;
	return sRet;
}
sal_Bool MixBulletsTypeMgr::IsCustomized(sal_uInt16 nIndex)
{
	sal_Bool bRet = sal_False;
	//sal_uInt16 nLength = 0;
	//nLength = sizeof(pActualBullets)/sizeof(BulletsSettings_Impl);

	if ( nIndex >= DEFAULT_BULLET_TYPES )
		bRet = sal_False;
	else
		bRet = pActualBullets[nIndex]->pBullets->bIsCustomized;

	return bRet;
}
/***************************************************************************************************
**********************Numbering Type lib**************************************************************
****************************************************************************************************/
NumberingTypeMgr* NumberingTypeMgr::_instance = 0;

NumberingTypeMgr::NumberingTypeMgr(const NBOType aType):
	NBOTypeMgrBase(aType),
	//pNumSettingsArr( new NumSettingsArr_Impl ),
	pNumberSettingsArr (new NumberSettingsArr_Impl)
{
	Init();
	pDefaultNumberSettingsArr = pNumberSettingsArr;
	pNumberSettingsArr = new NumberSettingsArr_Impl;
	// Initialize the first time to store the default value. Then do it again for customized value
	Init();
	ImplLoad(String::CreateFromAscii("standard.syb"));
}

NumberingTypeMgr::NumberingTypeMgr(const NBOType aType,const SfxItemSet* pArg):
	NBOTypeMgrBase(aType,pArg),
	//pNumSettingsArr( new NumSettingsArr_Impl ),
	pNumberSettingsArr (new NumberSettingsArr_Impl)
{
	Init();
	pDefaultNumberSettingsArr = pNumberSettingsArr;
	pNumberSettingsArr = new NumberSettingsArr_Impl;
	// Initialize the first time to store the default value. Then do it again for customized value
	Init();
	ImplLoad(String::CreateFromAscii("standard.syb"));
}

NumberingTypeMgr::NumberingTypeMgr(const NumberingTypeMgr& aTypeMgr):
	NBOTypeMgrBase(aTypeMgr),
	//pNumSettingsArr( new NumSettingsArr_Impl ),
	pNumberSettingsArr (new NumberSettingsArr_Impl)
{
	/*
	for(sal_uInt16 i=0;i<aTypeMgr.GetNumCount();i++)
	{
		NumberSettings_Impl* _pSet = aTypeMgr.GetNumSettingByIndex(i);
		if ( _pSet )
		{
			pNumberSettingsArr->GetObject(i)->nIndex = _pSet->nIndex;
			pNumberSettingsArr->GetObject(i)->nIndexDefault = _pSet->nIndexDefault;
			pNumberSettingsArr->GetObject(i)->sDescription = _pSet->sDescription;
			pNumberSettingsArr->GetObject(i)->bIsCustomized = _pSet->bIsCustomized;
			if ( _pSet->pNumSetting )
			{
				pNumberSettingsArr->GetObject(i)->pNumSetting->nNumberType = _pSet->pNumSetting->nNumberType;
				pNumberSettingsArr->GetObject(i)->pNumSetting->nParentNumbering = _pSet->pNumSetting->nParentNumbering;
				pNumberSettingsArr->GetObject(i)->pNumSetting->sPrefix = _pSet->pNumSetting->sPrefix;
				pNumberSettingsArr->GetObject(i)->pNumSetting->sSuffix = _pSet->pNumSetting->sSuffix;
				pNumberSettingsArr->GetObject(i)->pNumSetting->sBulletChar = _pSet->pNumSetting->sBulletChar;
				pNumberSettingsArr->GetObject(i)->pNumSetting->sBulletFont = _pSet->pNumSetting->sBulletFont;

				pNumberSettingsArr->GetObject(i)->pNumSetting->eLabelFollowedBy = _pSet->pNumSetting->eLabelFollowedBy;
				pNumberSettingsArr->GetObject(i)->pNumSetting->nTabValue = _pSet->pNumSetting->nTabValue;
				pNumberSettingsArr->GetObject(i)->pNumSetting->eNumAlign = _pSet->pNumSetting->eNumAlign;
				pNumberSettingsArr->GetObject(i)->pNumSetting->nNumAlignAt = _pSet->pNumSetting->nNumAlignAt;
				pNumberSettingsArr->GetObject(i)->pNumSetting->nNumIndentAt = _pSet->pNumSetting->nNumIndentAt;
			}
		}
	}
	*/
	ImplLoad(String::CreateFromAscii("standard.syb"));
}

void NumberingTypeMgr::Init()
{
	Reference< XMultiServiceFactory > xMSF = ::comphelper::getProcessServiceFactory();
	Reference < XInterface > xI = xMSF->createInstance(
		::rtl::OUString::createFromAscii( "com.sun.star.text.DefaultNumberingProvider" ) );
	Reference<XDefaultNumberingProvider> xDefNum(xI, UNO_QUERY);

	if(xDefNum.is())
	{
		Sequence< Sequence< PropertyValue > > aNumberings;
			LanguageType eLang = Application::GetSettings().GetLanguage();
		Locale aLocale = SvxCreateLocale(eLang);
		try
		{
			aNumberings = xDefNum->getDefaultContinuousNumberingLevels( aLocale );

				sal_Int32 nLength = aNumberings.getLength() > DEFAULT_NUM_VALUSET_COUNT ? DEFAULT_NUM_VALUSET_COUNT :aNumberings.getLength();

			const Sequence<PropertyValue>* pValuesArr = aNumberings.getConstArray();
			for(sal_Int32 i = 0; i < nLength; i++)
			{
				NumSettings_ImplPtr pNew = lcl_CreateNumberingSettingsPtr(pValuesArr[i]);
				NumberSettings_Impl* pNumEntry = new NumberSettings_Impl;
				pNumEntry->nIndex = i + 1;
				pNumEntry->nIndexDefault = i;
				pNumEntry->pNumSetting = pNew;
				//SetItemText( i + 1, SVX_RESSTR( RID_SVXSTR_SINGLENUM_DESCRIPTIONS + i ));
//				{
//					String sText;
//					//const OUString sValue(C2U("Value"));
//					Reference<XNumberingFormatter> xFormatter(xDefNum, UNO_QUERY);
//					if(xFormatter.is() && aNumberings.getLength() > i)
//					{

//						for (sal_uInt16 j=0;j<3;j++)
//						{
//							Sequence<PropertyValue> aLevel = aNumberings.getConstArray()[i];
//							try
//							{
//								aLevel.realloc(aLevel.getLength() + 1);
//								PropertyValue& rValue = aLevel.getArray()[aLevel.getLength() - 1];
//								rValue.Name = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Value"));
//								rValue.Value <<= (sal_Int32)(j + 1);

//								if (j!=0)
//									sText += String::CreateFromAscii(" ");

//								sText+=String(xFormatter->makeNumberingString( aLevel, aLocale ));
//							}
//							catch(Exception&)
//							{
//								DBG_ERROR("Exception in DefaultNumberingProvider::makeNumberingString");
//							}
//						}
//					}
//					String aStrFromRES(SVX_RESSTR( RID_SVXSTR_SINGLENUM_DESCRIPTIONS));
//					String aReplace = String::CreateFromAscii("%NUMBERINGSAMPLE");
//					aStrFromRES.SearchAndReplace(aReplace,sText);
//					pNumEntry->sDescription = aStrFromRES;
//				}
		// End modification

				pNumEntry->sDescription = SVX_RESSTR( RID_SVXSTR_SINGLENUM_DESCRIPTIONS + i );
				pNumberSettingsArr->Insert(pNumEntry, pNumberSettingsArr->Count());
			}
		}
		catch(Exception&)
		{
		}
	}
}

sal_uInt16 NumberingTypeMgr::GetNBOIndexForNumRule(SvxNumRule& aNum,sal_uInt16 mLevel,sal_uInt16 nFromIndex)
{
	if ( mLevel == (sal_uInt16)0xFFFF || mLevel > aNum.GetLevelCount() || mLevel == 0)
		return (sal_uInt16)0xFFFF;

	sal_uInt16 nActLv = IsSingleLevel(mLevel);

	if ( nActLv == (sal_uInt16)0xFFFF )
		return (sal_uInt16)0xFFFF;

	SvxNumberFormat aFmt(aNum.GetLevel(nActLv));
	String sPreFix = aFmt.GetPrefix();
	String sSuffix = aFmt.GetSuffix();
	String sEmpty;
		sal_Int16 eNumType = aFmt.GetNumberingType();

	sal_uInt16 nCount = pNumberSettingsArr->Count();
	for(sal_uInt16 i = nFromIndex; i < nCount; i++)
	{
		NumberSettings_ImplPtr _pSet = pNumberSettingsArr->GetObject(i);
		sal_Int16 eNType = _pSet->pNumSetting->nNumberType;
		String sLocalPreFix = _pSet->pNumSetting->sPrefix.getStr();
		String sLocalSuffix = _pSet->pNumSetting->sSuffix.getStr();
		if (sPreFix.CompareTo(sLocalPreFix)==COMPARE_EQUAL &&
			sSuffix.CompareTo(sLocalSuffix)==COMPARE_EQUAL &&
			eNumType == eNType )
		{
			return i+1;
		}
	}


	return (sal_uInt16)0xFFFF;
}

sal_Bool NumberingTypeMgr::RelplaceNumRule(SvxNumRule& aNum,sal_uInt16 nIndex,sal_uInt16 mLevel)
{
	//if ( mLevel == (sal_uInt16)0xFFFF || mLevel == 0)
	//	return sal_False;

	//if ( GetNBOIndexForNumRule(aNum,mLevel) != (sal_uInt16)0xFFFF )
	//	return sal_False;

	sal_uInt16 nActLv = IsSingleLevel(mLevel);

	if ( nActLv == (sal_uInt16)0xFFFF )
		return sal_False;

	SvxNumberFormat aFmt(aNum.GetLevel(nActLv));
	//sal_Unicode cPrefix = rtl::OUString(aFmt.GetPrefix()).getStr()[0];
	//sal_Unicode cSuffix = rtl::OUString(aFmt.GetSuffix()).getStr()[0];
	sal_Int16 eNumType = aFmt.GetNumberingType();

	sal_uInt16 nCount = pNumberSettingsArr->Count();
	if ( nIndex >= nCount )
		return sal_False;

	NumberSettings_ImplPtr _pSet = pNumberSettingsArr->GetObject(nIndex);

	_pSet->pNumSetting->sPrefix = aFmt.GetPrefix();
	_pSet->pNumSetting->sSuffix = aFmt.GetSuffix();
	 _pSet->pNumSetting->nNumberType = eNumType;
	_pSet->bIsCustomized = sal_True;

	SvxNumRule aTmpRule1(aNum);
	SvxNumRule aTmpRule2(aNum);
	ApplyNumRule(aTmpRule1,nIndex,mLevel,true);
	ApplyNumRule(aTmpRule2,nIndex,mLevel,false);
	if (aTmpRule1==aTmpRule2) _pSet->bIsCustomized=false;
	if (_pSet->bIsCustomized) {
		String aStrFromRES = String(SVX_RESSTR( RID_SVXSTR_NUMBULLET_CUSTOM_NUMBERING_DESCRIPTION));
		String aReplace = String::CreateFromAscii("%LIST_NUM");
		String sNUM = String::CreateFromInt32( nIndex + 1 );
		aStrFromRES.SearchAndReplace(aReplace,sNUM);
		_pSet->sDescription = aStrFromRES;
	} else {
		_pSet->sDescription = GetDescription(nIndex,true);
	}
	ImplStore(String::CreateFromAscii("standard.syb"));
	return sal_True;
}

sal_Bool NumberingTypeMgr::ApplyNumRule(SvxNumRule& aNum,sal_uInt16 nIndex,sal_uInt16 mLevel,sal_Bool isDefault,sal_Bool isResetSize)
{
	//if ( mLevel == (sal_uInt16)0xFFFF )
	//	return sal_False;

	//DBG_ASSERT(pNumSettingsArr->Count() > nIndex, "wrong index");
	if(pNumberSettingsArr->Count() <= nIndex)
		return sal_False;
	NumberSettingsArr_Impl*		pCurrentNumberSettingsArr=pNumberSettingsArr;
	if (isDefault) pCurrentNumberSettingsArr=pDefaultNumberSettingsArr;
	NumberSettings_ImplPtr _pSet = pCurrentNumberSettingsArr->GetObject(nIndex);
	sal_Int16 eNewType = _pSet->pNumSetting->nNumberType;

	sal_uInt16 nMask = 1;
	String sNumCharFmtName = GetBulCharFmtName();
	for(sal_uInt16 i = 0; i < aNum.GetLevelCount(); i++)
	{
		if(mLevel & nMask)
		{
			SvxNumberFormat aFmt(aNum.GetLevel(i));
			if (eNewType!=aFmt.GetNumberingType()) isResetSize=true;
			aFmt.SetNumberingType(eNewType);
			aFmt.SetPrefix(_pSet->pNumSetting->sPrefix);
			aFmt.SetSuffix(_pSet->pNumSetting->sSuffix);

			aFmt.SetCharFmtName(sNumCharFmtName);
			if (isResetSize) aFmt.SetBulletRelSize(100);
			aNum.SetLevel(i, aFmt);
		}
		nMask <<= 1 ;
	}

	return sal_True;
}
String NumberingTypeMgr::GetDescription(sal_uInt16 nIndex,sal_Bool isDefault)
{
	String sRet;
	sal_uInt16 nLength = 0;
	nLength = pNumberSettingsArr->Count();

	if ( nIndex >= nLength )
		return sRet;
	else
		sRet = pNumberSettingsArr->GetObject(nIndex)->sDescription;
	if (isDefault) sRet = pDefaultNumberSettingsArr->GetObject(nIndex)->sDescription;

	return sRet;
}
sal_Bool NumberingTypeMgr::IsCustomized(sal_uInt16 nIndex)
{
	sal_Bool bRet = sal_False;
	sal_uInt16 nLength = 0;
	nLength = pNumberSettingsArr->Count();

	if ( nIndex >= nLength )
		bRet = sal_False;
	else
		bRet = pNumberSettingsArr->GetObject(nIndex)->bIsCustomized;

	return bRet;
}
sal_uInt16 NumberingTypeMgr::GetNumCount() const
{
	sal_uInt16 nRet = 0;
	if ( pNumberSettingsArr )
		nRet = pNumberSettingsArr->Count();

	return nRet;
}
NumberSettings_Impl* NumberingTypeMgr::GetNumSettingByIndex(sal_uInt16 nIndex) const
{
	NumberSettings_Impl* pRet = 0;
	if ( pNumberSettingsArr && nIndex< pNumberSettingsArr->Count() )
	{
		pRet = pNumberSettingsArr->GetObject(nIndex);
	}

	return pRet;
}
/***************************************************************************************************
**********************Multi-level /Outline Type lib*******************************************************
****************************************************************************************************/
OutlineTypeMgr* OutlineTypeMgr::_instance = 0;

OutlineTypeMgr::OutlineTypeMgr(const NBOType aType):
	NBOTypeMgrBase(aType)//,
	//pNumSettingsArrs( new NumSettingsArr_Impl[DEFAULT_NUM_VALUSET_COUNT] )
{
	Init();
	for(sal_Int32 nItem = 0; nItem < DEFAULT_NUM_VALUSET_COUNT; nItem++ )
	{
		pDefaultOutlineSettingsArrs[nItem] = pOutlineSettingsArrs[nItem];
	}
	// Initialize the first time to store the default value. Then do it again for customized value
	Init();
	ImplLoad(String::CreateFromAscii("standard.syc"));
}

OutlineTypeMgr::OutlineTypeMgr(const NBOType aType,const SfxItemSet* pArg):
	NBOTypeMgrBase(aType,pArg)//,
	//pNumSettingsArrs( new NumSettingsArr_Impl[DEFAULT_NUM_VALUSET_COUNT])
{
	Init();
	for(sal_Int32 nItem = 0; nItem < DEFAULT_NUM_VALUSET_COUNT; nItem++ )
	{
		pDefaultOutlineSettingsArrs[nItem] = pOutlineSettingsArrs[nItem];
	}
	// Initialize the first time to store the default value. Then do it again for customized value
	Init();
	ImplLoad(String::CreateFromAscii("standard.syc"));
}

OutlineTypeMgr::OutlineTypeMgr(const OutlineTypeMgr& aTypeMgr):
	NBOTypeMgrBase(aTypeMgr)//,
	//pNumSettingsArrs( new NumSettingsArr_Impl[DEFAULT_NUM_VALUSET_COUNT])
{
	Init();
	for(sal_Int32 nItem = 0; nItem < DEFAULT_NUM_VALUSET_COUNT; nItem++ )
	{
		pDefaultOutlineSettingsArrs[nItem] = pOutlineSettingsArrs[nItem];
	}
	// Initialize the first time to store the default value. Then do it again for customized value
	Init();
	ImplLoad(String::CreateFromAscii("standard.syc"));
}

void OutlineTypeMgr::Init()
{
	Reference< XMultiServiceFactory > xMSF = ::comphelper::getProcessServiceFactory();
	Reference < XInterface > xI = xMSF->createInstance(
		::rtl::OUString::createFromAscii( "com.sun.star.text.DefaultNumberingProvider" ) );
	Reference<XDefaultNumberingProvider> xDefNum(xI, UNO_QUERY);

	if(xDefNum.is())
	{
		Sequence<Reference<XIndexAccess> > aOutlineAccess;
			LanguageType eLang = Application::GetSettings().GetLanguage();
		Locale aLocale = SvxCreateLocale(eLang);
		try
		{
			aOutlineAccess = xDefNum->getDefaultOutlineNumberings( aLocale );

			SvxNumRule aDefNumRule( NUM_BULLET_REL_SIZE|NUM_CONTINUOUS|NUM_BULLET_COLOR|NUM_CHAR_TEXT_DISTANCE|NUM_SYMBOL_ALIGNMENT,10, sal_False ,
				SVX_RULETYPE_NUMBERING,SvxNumberFormat::LABEL_ALIGNMENT);

			for(sal_Int32 nItem = 0;
				nItem < aOutlineAccess.getLength() && nItem < DEFAULT_NUM_VALUSET_COUNT;
				nItem++ )
			{
				pOutlineSettingsArrs[ nItem ] = new OutlineSettings_Impl;
				OutlineSettings_Impl* pItemArr = pOutlineSettingsArrs[ nItem ];
				pItemArr->sDescription = SVX_RESSTR( RID_SVXSTR_OUTLINENUM_DESCRIPTION_0 + nItem );
				pItemArr->pNumSettingsArr = new NumSettingsArr_Impl;
				Reference<XIndexAccess> xLevel = aOutlineAccess.getConstArray()[nItem];
				for(sal_Int32 nLevel = 0; nLevel < xLevel->getCount() && nLevel < 5; nLevel++)
				{
					Any aValueAny = xLevel->getByIndex(nLevel);
					Sequence<PropertyValue> aLevelProps;
					aValueAny >>= aLevelProps;
					NumSettings_ImplPtr pNew = lcl_CreateNumberingSettingsPtr(aLevelProps);
					SvxNumberFormat aNumFmt( aDefNumRule.GetLevel( nLevel) );
					pNew->eLabelFollowedBy = aNumFmt.GetLabelFollowedBy();
					pNew->nTabValue = aNumFmt.GetListtabPos();
					pNew->eNumAlign = aNumFmt.GetNumAdjust();
					pNew->nNumAlignAt = aNumFmt.GetFirstLineIndent();
					pNew->nNumIndentAt = aNumFmt.GetIndentAt();
					pItemArr->pNumSettingsArr->Insert( pNew, pItemArr->pNumSettingsArr->Count() );
				}
			}
		}
		catch(Exception&)
		{
		}
	}
}

sal_uInt16 OutlineTypeMgr::GetNBOIndexForNumRule(SvxNumRule& aNum,sal_uInt16 /* mLevel */,sal_uInt16 nFromIndex)
{
	sal_uInt16 nLength = sizeof(pOutlineSettingsArrs)/sizeof(OutlineSettings_Impl*);
	for(sal_uInt16 iDex = nFromIndex; iDex < nLength; iDex++)
	{
		sal_Bool bNotMatch = sal_False;
		OutlineSettings_Impl* pItemArr = pOutlineSettingsArrs[iDex];
		sal_uInt16 nCount = pItemArr->pNumSettingsArr->Count();
		for (sal_uInt16 iLevel=0;iLevel < nCount;iLevel++)
		{
			NumSettings_ImplPtr _pSet = pItemArr->pNumSettingsArr->GetObject(iLevel);
			sal_Int16 eNType = _pSet->nNumberType;

				SvxNumberFormat aFmt(aNum.GetLevel(iLevel));
			String sPreFix = aFmt.GetPrefix();
			String sSuffix = aFmt.GetSuffix();
			String sEmpty;
				sal_Int16 eNumType = aFmt.GetNumberingType();
				if( eNumType == SVX_NUM_CHAR_SPECIAL)
			{
				sal_Unicode cChar = aFmt.GetBulletChar();
				sal_Unicode ccChar = _pSet->sBulletChar.getStr()[0];
				rtl::OUString sFont = _pSet->sBulletFont;
				if ( !((cChar == ccChar) && //pFont && sFont.compareTo(pFont->GetName()) &&
					_pSet->eLabelFollowedBy == aFmt.GetLabelFollowedBy() &&
					_pSet->nTabValue == aFmt.GetListtabPos() &&
					_pSet->eNumAlign == aFmt.GetNumAdjust() &&
					_pSet->nNumAlignAt == aFmt.GetFirstLineIndent() &&
					_pSet->nNumIndentAt == aFmt.GetIndentAt()))
				{
					bNotMatch = sal_True;
					break;
				}
				}else if ((eNumType&(~LINK_TOKEN)) == SVX_NUM_BITMAP ) {
						const SvxBrushItem* pBrsh1 = aFmt.GetBrush();
						const SvxBrushItem* pBrsh2 = _pSet->pBrushItem;
						sal_Bool bIsMatch = false;
						if (pBrsh1==pBrsh2) bIsMatch = true;
						if (pBrsh1 && pBrsh2) {
							const Graphic* pGrf1 = pBrsh1->GetGraphic();;
							const Graphic* pGrf2 = pBrsh2->GetGraphic();;
							if (pGrf1==pGrf2) bIsMatch = true;
							if (pGrf1 && pGrf2) {
								if ( pGrf1->GetBitmap().IsEqual(pGrf2->GetBitmap()) &&
									 _pSet->aSize==aFmt.GetGraphicSize())
									bIsMatch = true;
							}
						}
						if (!bIsMatch) {
							bNotMatch = sal_True;
							break;
						}
				} else
			{
				if (!((sPreFix.CompareTo(_pSet->sPrefix.getStr())==COMPARE_EQUAL) &&
					( sSuffix.CompareTo(_pSet->sSuffix.getStr())==COMPARE_EQUAL ) &&
					eNumType == eNType &&
					_pSet->eLabelFollowedBy == aFmt.GetLabelFollowedBy() &&
					_pSet->nTabValue == aFmt.GetListtabPos() &&
					_pSet->eNumAlign == aFmt.GetNumAdjust() &&
					_pSet->nNumAlignAt == aFmt.GetFirstLineIndent() &&
					_pSet->nNumIndentAt == aFmt.GetIndentAt()))
				{
					bNotMatch = sal_True;
					break;
				}
			}
		}
		if ( !bNotMatch )
			return iDex+1;
	}


	return (sal_uInt16)0xFFFF;
}

sal_Bool OutlineTypeMgr::RelplaceNumRule(SvxNumRule& aNum,sal_uInt16 nIndex,sal_uInt16 mLevel)
{
	//if ( mLevel == 0 || mLevel == (sal_uInt16)0xFFFF )
	//	return sal_False;

	sal_uInt16 nLength = sizeof(pOutlineSettingsArrs)/sizeof(OutlineSettings_Impl*);
	if ( nIndex >= nLength )
		return sal_False;

	OutlineSettings_Impl* pItemArr = pOutlineSettingsArrs[nIndex];
	sal_uInt16 nCount = pItemArr->pNumSettingsArr->Count();
	for (sal_uInt16 iLevel=0;iLevel < nCount;iLevel++)
	{
		SvxNumberFormat aFmt(aNum.GetLevel(iLevel));
		sal_Int16 eNumType = aFmt.GetNumberingType();

		NumSettings_ImplPtr _pSet = pItemArr->pNumSettingsArr->GetObject(iLevel);

		_pSet->eLabelFollowedBy = aFmt.GetLabelFollowedBy();
		_pSet->nTabValue = aFmt.GetListtabPos();
		_pSet->eNumAlign = aFmt.GetNumAdjust();
		_pSet->nNumAlignAt = aFmt.GetFirstLineIndent();
		_pSet->nNumIndentAt = aFmt.GetIndentAt();

		if( eNumType == SVX_NUM_CHAR_SPECIAL)
		{
			sal_Unicode cChar = aFmt.GetBulletChar();
			OUString sChar(cChar);
			_pSet->sBulletChar = sChar;//OUString(cChar);
			if ( aFmt.GetBulletFont() )
				_pSet->sBulletFont = rtl::OUString(aFmt.GetBulletFont()->GetName());
			_pSet->nNumberType = eNumType;
			pItemArr->bIsCustomized = sal_True;
		}else if ((eNumType&(~LINK_TOKEN)) == SVX_NUM_BITMAP ) {
			if (_pSet->pBrushItem) {
				delete (_pSet->pBrushItem);
				_pSet->pBrushItem=NULL;
			}
			if (aFmt.GetBrush())
				_pSet->pBrushItem = new SvxBrushItem(*aFmt.GetBrush());
			_pSet->aSize = aFmt.GetGraphicSize();
			_pSet->nNumberType = eNumType;
		} else
		{
			_pSet->sPrefix = aFmt.GetPrefix();
			_pSet->sSuffix = aFmt.GetSuffix();
			_pSet->nNumberType = eNumType;
			if ( aFmt.GetBulletFont() )
				_pSet->sBulletFont = rtl::OUString(aFmt.GetBulletFont()->GetName());
			pItemArr->bIsCustomized = sal_True;
		}
	}
	SvxNumRule aTmpRule1(aNum);
	SvxNumRule aTmpRule2(aNum);
	ApplyNumRule(aTmpRule1,nIndex,mLevel,true);
	ApplyNumRule(aTmpRule2,nIndex,mLevel,false);
	if (aTmpRule1==aTmpRule2) pItemArr->bIsCustomized=false;
	if (pItemArr->bIsCustomized) {
		String aStrFromRES = String(SVX_RESSTR( RID_SVXSTR_NUMBULLET_CUSTOM_MULTILEVEL_DESCRIPTION));
		String aReplace = String::CreateFromAscii("%LIST_NUM");
		String sNUM = String::CreateFromInt32( nIndex + 1 );
		aStrFromRES.SearchAndReplace(aReplace,sNUM);
		pItemArr->sDescription = aStrFromRES;
	} else {
		pItemArr->sDescription = GetDescription(nIndex,true);
	}
	ImplStore(String::CreateFromAscii("standard.syc"));
	return sal_True;
}

sal_Bool OutlineTypeMgr::ApplyNumRule(SvxNumRule& aNum,sal_uInt16 nIndex,sal_uInt16 /* mLevel */,sal_Bool isDefault,sal_Bool isResetSize)
{
	//if ( mLevel == (sal_uInt16)0xFFFF )
	//	return sal_False;

	DBG_ASSERT(DEFAULT_NUM_VALUSET_COUNT > nIndex, "wrong index");
	if(DEFAULT_NUM_VALUSET_COUNT <= nIndex)
		return sal_False;

	const FontList* pList = 0;

	OutlineSettings_Impl* pItemArr = pOutlineSettingsArrs[nIndex];
	if (isDefault) pItemArr=pDefaultOutlineSettingsArrs[nIndex];

	//Font& rActBulletFont = lcl_GetDefaultBulletFont();
	NumSettingsArr_Impl *pNumSettingsArr=pItemArr->pNumSettingsArr;

	NumSettings_ImplPtr pLevelSettings = 0;
	String sBulletCharFmtName = GetBulCharFmtName();
	for(sal_uInt16 i = 0; i < aNum.GetLevelCount(); i++)
	{
		if(pNumSettingsArr->Count() > i)
			pLevelSettings = pNumSettingsArr->GetObject(i);

		if(!pLevelSettings)
			break;

		SvxNumberFormat aFmt(aNum.GetLevel(i));
		//aFmt.SetBulletFont(&pLevelSettings->aFont);
		Font& rActBulletFont = lcl_GetDefaultBulletFont();
		if (pLevelSettings->nNumberType !=aFmt.GetNumberingType()) isResetSize=true;
		aFmt.SetNumberingType( pLevelSettings->nNumberType );
		sal_uInt16 nUpperLevelOrChar = (sal_uInt16)pLevelSettings->nParentNumbering;
		if(aFmt.GetNumberingType() == SVX_NUM_CHAR_SPECIAL)
		{
			if( pLevelSettings->sBulletFont.getLength() &&
				pLevelSettings->sBulletFont.compareTo(rActBulletFont.GetName()))
			{
				// search for the font
				if(!pList)
				{
					SfxObjectShell* pCurDocShell = SfxObjectShell::Current();
					const SvxFontListItem* pFontListItem = (const SvxFontListItem* )pCurDocShell->GetItem( SID_ATTR_CHAR_FONTLIST );
					pList = pFontListItem ? pFontListItem->GetFontList() : 0;
				}
				if(pList && pList->IsAvailable( pLevelSettings->sBulletFont ) )
				{
			FontInfo aInfo = pList->Get(pLevelSettings->sBulletFont,WEIGHT_NORMAL, ITALIC_NONE);
			Font aFont(aInfo);
			aFmt.SetBulletFont(&aFont);
			}
				else
				{
					// if it cannot be found then create a new one
					Font aCreateFont( pLevelSettings->sBulletFont,String(), Size( 0, 14 ) );
					aCreateFont.SetCharSet( RTL_TEXTENCODING_DONTKNOW );
					aCreateFont.SetFamily( FAMILY_DONTKNOW );
					aCreateFont.SetPitch( PITCH_DONTKNOW );
					aCreateFont.SetWeight( WEIGHT_DONTKNOW );
					aCreateFont.SetTransparent( sal_True );
					aFmt.SetBulletFont( &aCreateFont );
				}
			}else
			aFmt.SetBulletFont( &rActBulletFont );

			sal_Unicode cChar = 0;
			if( pLevelSettings->sBulletChar.getLength() )
				cChar = pLevelSettings->sBulletChar.getStr()[0];
			if( Application::GetSettings().GetLayoutRTL() )
			{
					if( 0 == i && cChar == BulletsTypeMgr::aDynamicBulletTypes[5] )
					cChar = BulletsTypeMgr::aDynamicRTLBulletTypes[5];
				else if( 1 == i )
				{
					const SvxNumberFormat& numberFmt = aNum.GetLevel(0);
					if( numberFmt.GetBulletChar() == BulletsTypeMgr::aDynamicRTLBulletTypes[5] )
						cChar = BulletsTypeMgr::aDynamicRTLBulletTypes[4];
				}
			}

			aFmt.SetBulletChar(cChar);
			aFmt.SetCharFmtName( sBulletCharFmtName );
			if (isResetSize) aFmt.SetBulletRelSize(45);
		}else if ((aFmt.GetNumberingType()&(~LINK_TOKEN)) == SVX_NUM_BITMAP ) {
			if (pLevelSettings->pBrushItem) {
					const Graphic* pGrf = pLevelSettings->pBrushItem->GetGraphic();;
					Size aSize = pLevelSettings->aSize;
					sal_Int16 eOrient = text::VertOrientation::LINE_CENTER;
					if (!isResetSize && aFmt.GetGraphicSize()!=Size(0,0)) aSize=aFmt.GetGraphicSize();
					else {
						if (aSize.Width()==0 && aSize.Height()==0 && pGrf) {
							aSize = SvxNumberFormat::GetGraphicSizeMM100( pGrf );
						}
					}
					aSize = OutputDevice::LogicToLogic(aSize, MAP_100TH_MM, (MapUnit)GetMapUnit());
					aFmt.SetGraphicBrush( pLevelSettings->pBrushItem, &aSize, &eOrient );
			}
		} else
		{
			aFmt.SetIncludeUpperLevels(sal::static_int_cast< sal_uInt8 >(0 != nUpperLevelOrChar ? aNum.GetLevelCount() : 0));
			aFmt.SetCharFmtName(sBulletCharFmtName);
			if (isResetSize) aFmt.SetBulletRelSize(100);
		}
		if(pNumSettingsArr->Count() > i) {
			aFmt.SetLabelFollowedBy(pLevelSettings->eLabelFollowedBy);
			aFmt.SetListtabPos(pLevelSettings->nTabValue);
			aFmt.SetNumAdjust(pLevelSettings->eNumAlign);
			aFmt.SetFirstLineIndent(pLevelSettings->nNumAlignAt);
			aFmt.SetIndentAt(pLevelSettings->nNumIndentAt);
		}
		aFmt.SetPrefix(pLevelSettings->sPrefix);
		aFmt.SetSuffix(pLevelSettings->sSuffix);
		aNum.SetLevel(i, aFmt);
	}

	return sal_True;
}
String OutlineTypeMgr::GetDescription(sal_uInt16 nIndex,sal_Bool isDefault)
{
	String sRet;
	sal_uInt16 nLength = 0;
	nLength = sizeof(pOutlineSettingsArrs)/sizeof(OutlineSettings_Impl*);

	if ( nIndex >= nLength )
		return sRet;
	else
	{
		OutlineSettings_Impl* pItemArr = pOutlineSettingsArrs[nIndex];
		if (isDefault) pItemArr = pDefaultOutlineSettingsArrs[nIndex];
		if ( pItemArr )
		{
			sRet = pItemArr->sDescription;
		};
	}
	return sRet;
}
sal_Bool OutlineTypeMgr::IsCustomized(sal_uInt16 nIndex)
{
	sal_Bool bRet = sal_False;

	sal_uInt16 nLength = 0;
	nLength = sizeof(pOutlineSettingsArrs)/sizeof(OutlineSettings_Impl*);

	if ( nIndex >= nLength )
		return bRet;
	else
	{
		OutlineSettings_Impl* pItemArr = pOutlineSettingsArrs[nIndex];
		if ( pItemArr )
		{
			bRet = pItemArr->bIsCustomized;
		};
	}

	return bRet;
}


}}

/* vim: set noet sw=4 ts=4: */
