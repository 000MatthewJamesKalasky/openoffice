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



#ifndef _SVX_ZOOMSLIDERITEM_HXX
#define _SVX_ZOOMSLIDERITEM_HXX

#include <svl/intitem.hxx>
#ifndef _SVX_SVXIDS_HRC
#include <svx/svxids.hrc>
#endif
#include <com/sun/star/uno/Sequence.hxx>
#include "svx/svxdllapi.h"

//-------------------------------------------------------------------------

class SVX_DLLPUBLIC SvxZoomSliderItem: public SfxUInt16Item
{
	com::sun::star::uno::Sequence < sal_Int32 > maValues;
	sal_uInt16 mnMinZoom;
	sal_uInt16 mnMaxZoom;

public:
	TYPEINFO();

	SvxZoomSliderItem( sal_uInt16 nCurrentZoom = 100, sal_uInt16 nMinZoom = 20, sal_uInt16 nMaxZoom = 600, sal_uInt16 nWhich = SID_ATTR_ZOOMSLIDER );
	SvxZoomSliderItem( const SvxZoomSliderItem& );
	~SvxZoomSliderItem();

	void AddSnappingPoint( sal_Int32 nNew );
	const com::sun::star::uno::Sequence < sal_Int32 >& GetSnappingPoints() const;
	sal_uInt16 GetMinZoom() const {return mnMinZoom;}
	sal_uInt16 GetMaxZoom() const {return mnMaxZoom;}

	// "pure virtual methods" from SfxPoolItem
	virtual int				operator==( const SfxPoolItem& ) const;
	virtual SfxPoolItem*	Clone( SfxItemPool *pPool = 0 ) const;
	virtual SfxPoolItem*	Create( SvStream& rStrm, sal_uInt16 nVersion ) const; // empty
	virtual SvStream&		Store( SvStream& rStrm , sal_uInt16 nItemVersion ) const; // empty
	virtual sal_Bool		QueryValue( com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const; // empty
	virtual sal_Bool		PutValue( const com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 ); // empty
};

//------------------------------------------------------------------------

#endif

/* vim: set noet sw=4 ts=4: */
