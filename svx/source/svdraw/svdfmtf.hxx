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



#ifndef _SVDFMTF_HXX
#define _SVDFMTF_HXX

#include <vcl/metaact.hxx>
#include <vcl/virdev.hxx>
#include <svx/svdobj.hxx>

//************************************************************
//   Vorausdeklarationen
//************************************************************

class SfxItemSet;
class SdrObjList;
class SdrModel;
class SdrPage;
class SdrObject;
class SvdProgressInfo;

//************************************************************
//   Hilfsklasse SdrObjRefList
//************************************************************

class SdrObjRefList
{
	Container					aList;
public:

	SdrObjRefList()
	:	aList(1024,64,64)
	{}

	void Clear() { aList.Clear(); }
	sal_uLong GetObjCount() const { return aList.Count(); }
	SdrObject* GetObj(sal_uLong nNum) const { return (SdrObject*)aList.GetObject(nNum); }
	SdrObject* operator[](sal_uLong nNum) const { return (SdrObject*)aList.GetObject(nNum); }
	void InsertObject(SdrObject* pObj, sal_uLong nPos=CONTAINER_APPEND) { aList.Insert(pObj,nPos); }
	void RemoveObject(sal_uLong nPos) { aList.Remove(nPos); }
};

//************************************************************
//   Hilfsklasse ImpSdrGDIMetaFileImport
//************************************************************

class ImpSdrGDIMetaFileImport
{
protected:
	SdrObjRefList				aTmpList;
	VirtualDevice				aVD;
	Rectangle					aScaleRect;
	sal_uLong						nMapScalingOfs; // ab hier nocht nicht mit MapScaling bearbeitet
	SfxItemSet*					pLineAttr;
	SfxItemSet*					pFillAttr;
	SfxItemSet*					pTextAttr;
	SdrPage*					pPage;
	SdrModel*					pModel;
	SdrLayerID					nLayer;
	Color						aOldLineColor;
	sal_Int32					nLineWidth;
	basegfx::B2DLineJoin		maLineJoin;
	com::sun::star::drawing::LineCap    maLineCap;
	XDash						maDash;

	sal_Bool					bMov;
	sal_Bool					bSize;
	Point						aOfs;
    double                      fScaleX;
    double                      fScaleY;
	Fraction					aScaleX;
	Fraction					aScaleY;

	sal_Bool                    bFntDirty;

	// fuer Optimierung von (PenNULL,Brush,DrawPoly),(Pen,BrushNULL,DrawPoly) -> aus 2 mach ein
	sal_Bool                    bLastObjWasPolyWithoutLine;
	sal_Bool                    bNoLine;
	sal_Bool                    bNoFill;

	// fuer Optimierung mehrerer Linien zu einer Polyline
	sal_Bool                    bLastObjWasLine;

    // clipregion
    basegfx::B2DPolyPolygon     maClip;

protected:
    // ckeck for clip and evtl. fill maClip
    void checkClip();
    bool isClip() const;

    // actions
	void DoAction(MetaPixelAction			& rAct);
	void DoAction(MetaPointAction			& rAct);
	void DoAction(MetaLineAction			& rAct);
	void DoAction(MetaRectAction			& rAct);
	void DoAction(MetaRoundRectAction		& rAct);
	void DoAction(MetaEllipseAction			& rAct);
	void DoAction(MetaArcAction				& rAct);
	void DoAction(MetaPieAction				& rAct);
	void DoAction(MetaChordAction			& rAct);
	void DoAction(MetaPolyLineAction		& rAct);
	void DoAction(MetaPolygonAction			& rAct);
	void DoAction(MetaPolyPolygonAction		& rAct);
	void DoAction(MetaTextAction			& rAct);
	void DoAction(MetaTextArrayAction		& rAct);
	void DoAction(MetaStretchTextAction		& rAct);
	void DoAction(MetaBmpAction				& rAct);
	void DoAction(MetaBmpScaleAction		& rAct);
	void DoAction(MetaBmpExAction			& rAct);
	void DoAction(MetaBmpExScaleAction		& rAct);
	void DoAction(MetaHatchAction			& rAct);
	void DoAction(MetaLineColorAction		& rAct);
	void DoAction(MetaMapModeAction			& rAct);
	void DoAction(MetaFillColorAction		& rAct) { rAct.Execute(&aVD); }
	void DoAction(MetaTextColorAction		& rAct) { rAct.Execute(&aVD); }
	void DoAction(MetaTextFillColorAction	& rAct) { rAct.Execute(&aVD); }
	void DoAction(MetaFontAction			& rAct) { rAct.Execute(&aVD); bFntDirty=sal_True; }
	void DoAction(MetaTextAlignAction		& rAct) { rAct.Execute(&aVD); bFntDirty=sal_True; }
	void DoAction(MetaClipRegionAction		& rAct) { rAct.Execute(&aVD); checkClip(); }
	void DoAction(MetaRasterOpAction		& rAct) { rAct.Execute(&aVD); }
	void DoAction(MetaPushAction			& rAct) { rAct.Execute(&aVD); checkClip(); }
	void DoAction(MetaPopAction				& rAct) { rAct.Execute(&aVD); bFntDirty=sal_True; checkClip(); }
	void DoAction(MetaMoveClipRegionAction	& rAct) { rAct.Execute(&aVD); checkClip(); }
	void DoAction(MetaISectRectClipRegionAction& rAct) { rAct.Execute(&aVD); checkClip(); }
	void DoAction(MetaISectRegionClipRegionAction& rAct) { rAct.Execute(&aVD); checkClip(); }
	void DoAction(MetaCommentAction& rAct, GDIMetaFile* pMtf);

	void ImportText( const Point& rPos, const XubString& rStr, const MetaAction& rAct );
	void SetAttributes(SdrObject* pObj, FASTBOOL bForceTextAttr=sal_False);
	void InsertObj( SdrObject* pObj, sal_Bool bScale = sal_True );
	void MapScaling();

	// #i73407# reformulation to use new B2DPolygon classes
	bool CheckLastLineMerge(const basegfx::B2DPolygon& rSrcPoly);
	bool CheckLastPolyLineAndFillMerge(const basegfx::B2DPolyPolygon& rPolyPolygon);

public:
	ImpSdrGDIMetaFileImport(SdrModel& rModel);
	~ImpSdrGDIMetaFileImport();
	sal_uLong DoImport(const GDIMetaFile& rMtf, SdrObjList& rDestList, sal_uLong nInsPos=CONTAINER_APPEND, SvdProgressInfo *pProgrInfo = NULL);
	void SetLayer(SdrLayerID nLay) { nLayer=nLay; }
	SdrLayerID GetLayer() const { return nLayer; }
	void SetScaleRect(const Rectangle& rRect) { aScaleRect=rRect; }
	const Rectangle& GetScaleRect() const { return aScaleRect; }
};

////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //_SVDFMTF_HXX
// eof
