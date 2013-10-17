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



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"

#include <svx/svdoedge.hxx>
#include <svx/xpool.hxx>
#include <svx/xpoly.hxx>
#include <svx/svdattrx.hxx>
#include <svx/svdpool.hxx>
#include <svx/svdmodel.hxx>
#include <svx/svdpage.hxx>
#include <svx/svdpagv.hxx>
#include <svx/svdview.hxx>
#include <svx/svddrag.hxx>
#include <svx/svddrgv.hxx>
#include "svddrgm1.hxx"
#include <svx/svdhdl.hxx>
#include <svx/svdtrans.hxx>
#include <svx/svdetc.hxx>
#include "svx/svdglob.hxx"   // StringCache
#include "svx/svdstr.hrc"    // Objektname
#include <svl/style.hxx>
#include <svl/smplhint.hxx>
#include <editeng/eeitem.hxx>
#include <svx/sdr/properties/connectorproperties.hxx>
#include <svx/sdr/contact/viewcontactofsdredgeobj.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <svx/svdlegacy.hxx>
#include <svx/sdrhittesthelper.hxx>
#include <svx/svdobj.hxx>

////////////////////////////////////////////////////////////////////////////////////////////////////

#define SDRESC_VERT (sdr::glue::GluePoint::ESCAPE_DIRECTION_TOP|sdr::glue::GluePoint::ESCAPE_DIRECTION_BOTTOM)
#define SDRESC_HORZ (sdr::glue::GluePoint::ESCAPE_DIRECTION_LEFT|sdr::glue::GluePoint::ESCAPE_DIRECTION_RIGHT)
#define SDRESC_ALL (SDRESC_HORZ|SDRESC_VERT)

SdrObjConnection::~SdrObjConnection()
{
}

void SdrObjConnection::ResetVars()
{
	mpConnectedSdrObject = 0;
	mnConnectorId = 0;
	mbBestConnection = true;
	mbBestVertex = true;
	mbAutoVertex = false;
}

bool SdrObjConnection::TakeGluePoint(sdr::glue::GluePoint& rGP) const
{
    bool bRet(false);

    if(mpConnectedSdrObject) 
    { 
        // Ein Obj muss schon angedockt sein!
        const sdr::glue::GluePointProvider& rProvider = mpConnectedSdrObject->GetGluePointProvider();

        if(mbAutoVertex) 
        {
            rGP = rProvider.getAutoGluePointByIndex(mnConnectorId);
            bRet = true;
        } 
        else 
        {

            if(rProvider.hasUserGluePoints()) 
            {
                const sdr::glue::GluePoint* pCandidate = rProvider.findUserGluePointByID(mnConnectorId);

                if(pCandidate)
                {
                    rGP = *pCandidate;
                    bRet = true;
                }
            }
        }
    }

    return bRet;
}

basegfx::B2DPoint& SdrEdgeInfoRec::ImpGetLineVersatzPoint(SdrEdgeLineCode eLineCode)
{
	switch (eLineCode) 
    {
		case OBJ1LINE2 : return aObj1Line2;
		case OBJ1LINE3 : return aObj1Line3;
		case OBJ2LINE2 : return aObj2Line2;
		case OBJ2LINE3 : return aObj2Line3;
		case MIDDLELINE: return aMiddleLine;
	} // switch
	return aMiddleLine;
}

sal_uInt32 SdrEdgeInfoRec::ImpGetPolyIdx(SdrEdgeLineCode eLineCode, sal_uInt32 nPointCount) const
{
    switch (eLineCode) 
    {
        case OBJ1LINE2 : return 1;
        case OBJ1LINE3 : return 2;
        case OBJ2LINE2 : return nPointCount - 3;
        case OBJ2LINE3 : return nPointCount - 4;
        case MIDDLELINE: return nMiddleLine;
    } // switch
    return 0;
}

bool SdrEdgeInfoRec::ImpIsHorzLine(SdrEdgeLineCode eLineCode, sal_uInt32 nPointCount) const
{
    sal_uInt32 nIdx(ImpGetPolyIdx(eLineCode, nPointCount));
    bool bHorz(0 == nAngle1 || 18000 == nAngle1);

    if(OBJ2LINE2 == eLineCode || OBJ2LINE3 == eLineCode) 
    {
        nIdx = nPointCount - nIdx;
        bHorz = (0 == nAngle2 || 18000 == nAngle2);
    }

    if(1 == (nIdx & 1)) 
    {
        bHorz = !bHorz;
    }

    return bHorz;
}

void SdrEdgeInfoRec::ImpSetLineVersatz(SdrEdgeLineCode eLineCode, sal_uInt32 nPointCount, long nVal)
{
	basegfx::B2DPoint& rPt=ImpGetLineVersatzPoint(eLineCode);
	
    if(ImpIsHorzLine(eLineCode, nPointCount)) 
    {
        rPt.setY(nVal);
    }
	else
    {
        rPt.setX(nVal);
    }
}

long SdrEdgeInfoRec::ImpGetLineVersatz(SdrEdgeLineCode eLineCode, sal_uInt32 nPointCount) const
{
	const basegfx::B2DPoint& rPt = ImpGetLineVersatzPoint(eLineCode);

    if(ImpIsHorzLine(eLineCode, nPointCount)) 
    {
        return rPt.getY();
    }
	else 
    {
        return rPt.getX();
    }
}

//////////////////////////////////////////////////////////////////////////////
// BaseProperties section

sdr::properties::BaseProperties* SdrEdgeObj::CreateObjectSpecificProperties()
{
	return new sdr::properties::ConnectorProperties(*this);
}

//////////////////////////////////////////////////////////////////////////////
// DrawContact section

sdr::contact::ViewContact* SdrEdgeObj::CreateObjectSpecificViewContact()
{
	return new sdr::contact::ViewContactOfSdrEdgeObj(*this);
}

//////////////////////////////////////////////////////////////////////////////
// GluePoint section

namespace sdr
{
    namespace glue
    {
        // SdrEdgeObj implements it's own GluePointProvider since
        // - it does not support UserGluePoints -> use GluePointProvider
        // - it supports AutoGluePoints, but different from standard -> replace
        //   AutoGluePoint methods

        class SdrEdgeObjGluePointProvider : public GluePointProvider
        {
        private:
            // we need a reference to the owner object, given as reference in
            // the constructor to express that it will always be set
            const SdrEdgeObj*           mpSource;

        protected:
            SdrEdgeObjGluePointProvider(const SdrEdgeObjGluePointProvider& rCandidate);
            virtual GluePointProvider& operator=(const GluePointProvider& rCandidate);

        public:
            // construction, destruction, copying
            SdrEdgeObjGluePointProvider(const SdrEdgeObj& rSource);
            virtual ~SdrEdgeObjGluePointProvider();

            // copying
            virtual GluePointProvider* Clone() const;

            // AutoGluePoint read access (read only)
            virtual sal_uInt32 getAutoGluePointCount() const;
            virtual GluePoint getAutoGluePointByIndex(sal_uInt32 nIndex) const;
        };

        SdrEdgeObjGluePointProvider::SdrEdgeObjGluePointProvider(const SdrEdgeObj& rSource)
        :   GluePointProvider(),
            mpSource(&rSource)
        {
        }

        SdrEdgeObjGluePointProvider::~SdrEdgeObjGluePointProvider()
        {
        }

        SdrEdgeObjGluePointProvider::SdrEdgeObjGluePointProvider(const SdrEdgeObjGluePointProvider& rCandidate)
        :   GluePointProvider(),
            mpSource(rCandidate.mpSource)
        {
        }

        GluePointProvider& SdrEdgeObjGluePointProvider::operator=(const GluePointProvider& rCandidate)
        {
            // call parent to copy UserGluePoints
            GluePointProvider::operator=(rCandidate);

            const SdrEdgeObjGluePointProvider* pSource = dynamic_cast< const SdrEdgeObjGluePointProvider* >(&rCandidate);

            if(pSource)
            {
                mpSource = pSource->mpSource;
            }

            return *this;
        }

        GluePointProvider* SdrEdgeObjGluePointProvider::Clone() const
        {
            return new SdrEdgeObjGluePointProvider(*this);
        }

        sal_uInt32 SdrEdgeObjGluePointProvider::getAutoGluePointCount() const
        {
            // to stay compatible, also use four points (anyways UNO API uses a fixed count of four
            // currently). Seel below, 0,1 create the same point, 2 the start, 3, the end
            return 4;
        }

        GluePoint SdrEdgeObjGluePointProvider::getAutoGluePointByIndex(sal_uInt32 nIndex) const
        {
            // no error with indices, just repeatedly return last GluePoint as fallback
            const basegfx::B2DPolygon aEdgeTrack(mpSource->GetEdgeTrackPath());
            const sal_uInt32 nPntAnz(aEdgeTrack.count());
            basegfx::B2DPoint aOldPoint(0.0, 0.0);

            if(nPntAnz)
            {
                switch(nIndex)
                {
                    case 2:
                    {
                        if(!mpSource->GetConnectedNode(true)) 
                        {
                            aOldPoint = aEdgeTrack.getB2DPoint(0);
                        }
                        break;
                    }
                    case 3:
                    {
                        if(!mpSource->GetConnectedNode(false)) 
                        {
                            aOldPoint = aEdgeTrack.getB2DPoint(nPntAnz - 1);
                        }
                        break;
                    }
                    default:
                    {
                        if(1 == (nPntAnz & 1)) 
                        {
                            aOldPoint = aEdgeTrack.getB2DPoint(nPntAnz / 2);
                        } 
                        else 
                        {
                            aOldPoint = (aEdgeTrack.getB2DPoint((nPntAnz/2) - 1) + aEdgeTrack.getB2DPoint(nPntAnz/2)) * 0.5;
                        }
                        break;
                    }
                }
            }

            // need to make absolute position relative. temporarily correct zero sizes for invert
            basegfx::B2DHomMatrix aTransform(basegfx::tools::guaranteeMinimalScaling(mpSource->getSdrObjectTransformation()));

            aTransform.invert();
            aOldPoint = aTransform * aOldPoint;

            return sdr::glue::GluePoint(
                aOldPoint,
                sdr::glue::GluePoint::ESCAPE_DIRECTION_SMART,
                sdr::glue::GluePoint::Alignment_Center,
                sdr::glue::GluePoint::Alignment_Center,
                true,   // mbRelative
                false); // mbUserDefined
        }
    } // end of namespace glue
} // end of namespace sdr

sdr::glue::GluePointProvider* SdrEdgeObj::CreateObjectSpecificGluePointProvider()
{
    return new sdr::glue::SdrEdgeObjGluePointProvider(*this);
}

//////////////////////////////////////////////////////////////////////////////

SdrEdgeObj::SdrEdgeObj(SdrModel& rSdrModel)
:	SdrTextObj(rSdrModel),
    maCon1(),
    maCon2(),
    maEdgeTrack(),
    maEdgeInfo(),
	mbEdgeTrackDirty(false),
	mbEdgeTrackUserDefined(false),
	mbSuppressDefaultConnect(false)
{
    // default connector
    maEdgeTrack.append(basegfx::B2DPoint(0.0, 0.0));
    maEdgeTrack.append(basegfx::B2DPoint(0.0, 100.0));
}

SdrEdgeObj::~SdrEdgeObj()
{
	DisconnectFromNode(true);
	DisconnectFromNode(false);
}

void SdrEdgeObj::copyDataFromSdrObject(const SdrObject& rSource)
{
	if(this != &rSource)
	{
		const SdrEdgeObj* pSource = dynamic_cast< const SdrEdgeObj* >(&rSource);

		if(pSource)
		{
			// call parent
			SdrTextObj::copyDataFromSdrObject(rSource);

			// copy local data
			maEdgeTrack = pSource->maEdgeTrack;
			mbEdgeTrackDirty = pSource->mbEdgeTrackDirty;
			maCon1 = pSource->maCon1;
			maCon2 = pSource->maCon2;
			maCon1.mpConnectedSdrObject = 0;
			maCon2.mpConnectedSdrObject = 0;
			maEdgeInfo = pSource->maEdgeInfo;
		}
		else
		{
			OSL_ENSURE(false, "copyDataFromSdrObject with ObjectType of Source different from Target (!)");
		}
	}
}

SdrObject* SdrEdgeObj::CloneSdrObject(SdrModel* pTargetModel) const
{
	SdrEdgeObj* pClone = new SdrEdgeObj(
		pTargetModel ? *pTargetModel : getSdrModelFromSdrObject());
	OSL_ENSURE(pClone, "CloneSdrObject error (!)");
	pClone->copyDataFromSdrObject(*this);

	return pClone;
}

bool SdrEdgeObj::IsClosedObj() const
{
	return false;
}

bool SdrEdgeObj::IsSdrEdgeObj() const 
{ 
    return true; 
}

void SdrEdgeObj::ImpSetAttrToEdgeInfo()
{
	const SfxItemSet& rSet = GetObjectItemSet();
	SdrEdgeKind eKind = ((SdrEdgeKindItem&)(rSet.Get(SDRATTR_EDGEKIND))).GetValue();
	sal_Int32 nVal1 = ((SdrEdgeLine1DeltaItem&)rSet.Get(SDRATTR_EDGELINE1DELTA)).GetValue();
	sal_Int32 nVal2 = ((SdrEdgeLine2DeltaItem&)rSet.Get(SDRATTR_EDGELINE2DELTA)).GetValue();
	sal_Int32 nVal3 = ((SdrEdgeLine3DeltaItem&)rSet.Get(SDRATTR_EDGELINE3DELTA)).GetValue();

	if(eKind == SDREDGE_ORTHOLINES || eKind == SDREDGE_BEZIER)
	{
		sal_Int32 nVals[3] = { nVal1, nVal2, nVal3 };
		sal_uInt16 n = 0;

		if(maEdgeInfo.nObj1Lines >= 2 && n < 3)
		{
			maEdgeInfo.ImpSetLineVersatz(OBJ1LINE2, maEdgeTrack.count(), nVals[n]);
			n++;
		}

		if(maEdgeInfo.nObj1Lines >= 3 && n < 3)
		{
			maEdgeInfo.ImpSetLineVersatz(OBJ1LINE3, maEdgeTrack.count(), nVals[n]);
			n++;
		}

		if(maEdgeInfo.nMiddleLine != 0xFFFF && n < 3)
		{
			maEdgeInfo.ImpSetLineVersatz(MIDDLELINE, maEdgeTrack.count(), nVals[n]);
			n++;
		}

		if(maEdgeInfo.nObj2Lines >= 3 && n < 3)
		{
			maEdgeInfo.ImpSetLineVersatz(OBJ2LINE3, maEdgeTrack.count(), nVals[n]);
			n++;
		}

		if(maEdgeInfo.nObj2Lines >= 2 && n < 3)
		{
			maEdgeInfo.ImpSetLineVersatz(OBJ2LINE2, maEdgeTrack.count(), nVals[n]);
			n++;
		}
	}
	else if(eKind == SDREDGE_THREELINES)
	{
		bool bHor1 = maEdgeInfo.nAngle1 == 0 || maEdgeInfo.nAngle1 == 18000;
		bool bHor2 = maEdgeInfo.nAngle2 == 0 || maEdgeInfo.nAngle2 == 18000;

		if(bHor1)
		{
			maEdgeInfo.aObj1Line2.setX(nVal1);
		}
		else
		{
			maEdgeInfo.aObj1Line2.setY(nVal1);
		}

		if(bHor2)
		{
			maEdgeInfo.aObj2Line2.setX(nVal2);
		}
		else
		{
			maEdgeInfo.aObj2Line2.setY(nVal2);
		}
	}

	// #84649#
	ImpDirtyEdgeTrack();
}

void SdrEdgeObj::ImpSetEdgeInfoToAttr()
{
	const SfxItemSet& rSet = GetObjectItemSet();
	SdrEdgeKind eKind = ((SdrEdgeKindItem&)(rSet.Get(SDRATTR_EDGEKIND))).GetValue();
	sal_Int32 nValAnz = ((SfxUInt16Item&)rSet.Get(SDRATTR_EDGELINEDELTAANZ)).GetValue();
	sal_Int32 nVal1 = ((SdrEdgeLine1DeltaItem&)rSet.Get(SDRATTR_EDGELINE1DELTA)).GetValue();
	sal_Int32 nVal2 = ((SdrEdgeLine2DeltaItem&)rSet.Get(SDRATTR_EDGELINE2DELTA)).GetValue();
	sal_Int32 nVal3 = ((SdrEdgeLine3DeltaItem&)rSet.Get(SDRATTR_EDGELINE3DELTA)).GetValue();
	sal_Int32 nVals[3] = { nVal1, nVal2, nVal3 };
	sal_uInt16 n = 0;

	if(eKind == SDREDGE_ORTHOLINES || eKind == SDREDGE_BEZIER)
	{
		if(maEdgeInfo.nObj1Lines >= 2 && n < 3)
		{
			nVals[n] = maEdgeInfo.ImpGetLineVersatz(OBJ1LINE2, maEdgeTrack.count());
			n++;
		}

		if(maEdgeInfo.nObj1Lines >= 3 && n < 3)
		{
			nVals[n] = maEdgeInfo.ImpGetLineVersatz(OBJ1LINE3, maEdgeTrack.count());
			n++;
		}

		if(maEdgeInfo.nMiddleLine != 0xFFFF && n < 3)
		{
			nVals[n] = maEdgeInfo.ImpGetLineVersatz(MIDDLELINE, maEdgeTrack.count());
			n++;
		}

		if(maEdgeInfo.nObj2Lines >= 3 && n < 3)
		{
			nVals[n] = maEdgeInfo.ImpGetLineVersatz(OBJ2LINE3, maEdgeTrack.count());
			n++;
		}

		if(maEdgeInfo.nObj2Lines >= 2 && n < 3)
		{
			nVals[n] = maEdgeInfo.ImpGetLineVersatz(OBJ2LINE2, maEdgeTrack.count());
			n++;
		}
	}
	else if(eKind == SDREDGE_THREELINES)
	{
		bool bHor1 = maEdgeInfo.nAngle1 == 0 || maEdgeInfo.nAngle1 == 18000;
		bool bHor2 = maEdgeInfo.nAngle2 == 0 || maEdgeInfo.nAngle2 == 18000;

		n = 2;
		nVals[0] = bHor1 ? maEdgeInfo.aObj1Line2.getX() : maEdgeInfo.aObj1Line2.getY();
		nVals[1] = bHor2 ? maEdgeInfo.aObj2Line2.getX() : maEdgeInfo.aObj2Line2.getY();
	}

	if(n != nValAnz || nVals[0] != nVal1 || nVals[1] != nVal2 || nVals[2] != nVal3)
	{
		// #75371# Here no more notifying is necessary, just local changes are OK.
		if(n != nValAnz)
		{
			GetProperties().SetObjectItemDirect(SfxUInt16Item(SDRATTR_EDGELINEDELTAANZ, n));
		}

		if(nVals[0] != nVal1)
		{
			GetProperties().SetObjectItemDirect(SdrEdgeLine1DeltaItem(nVals[0]));
		}

		if(nVals[1] != nVal2)
		{
			GetProperties().SetObjectItemDirect(SdrEdgeLine2DeltaItem(nVals[1]));
		}

		if(nVals[2] != nVal3)
		{
			GetProperties().SetObjectItemDirect(SdrEdgeLine3DeltaItem(nVals[2]));
		}

		if(n < 3)
		{
			GetProperties().ClearObjectItemDirect(SDRATTR_EDGELINE3DELTA);
		}

		if(n < 2)
		{
			GetProperties().ClearObjectItemDirect(SDRATTR_EDGELINE2DELTA);
		}

		if(n < 1)
		{
			GetProperties().ClearObjectItemDirect(SDRATTR_EDGELINE1DELTA);
		}
	}
}

void SdrEdgeObj::TakeObjInfo(SdrObjTransformInfoRec& rInfo) const
{
    // #54102# allow rotation, mirror and shear
	rInfo.mbRotateFreeAllowed = true;
	rInfo.mbRotate90Allowed = true;
	rInfo.mbMirrorFreeAllowed = true;
	rInfo.mbMirror45Allowed = true;
	rInfo.mbMirror90Allowed = true;
	rInfo.mbTransparenceAllowed = false;
	rInfo.mbGradientAllowed = false;
	rInfo.mbShearAllowed = true;
	rInfo.mbEdgeRadiusAllowed = false;
	const bool bCanConv(!HasText() || ImpCanConvTextToCurve());
	rInfo.mbCanConvToPath = bCanConv;
	rInfo.mbCanConvToPoly = bCanConv;
	rInfo.mbCanConvToContour = (rInfo.mbCanConvToPoly || LineGeometryUsageIsNecessary());
}

sal_uInt16 SdrEdgeObj::GetObjIdentifier() const
{
	return sal_uInt16(OBJ_EDGE);
}

void SdrEdgeObj::ConnectToNode(bool bTail1, SdrObject* pObj)
{
	SdrObjConnection& rCon=GetConnection(bTail1);
	DisconnectFromNode(bTail1);
	
	if(pObj) 
	{
		StartListening(*pObj);
		rCon.mpConnectedSdrObject = pObj;
        
        // #120437# If connection is set, reset bEdgeTrackUserDefined
	    mbEdgeTrackUserDefined = false;

        ImpDirtyEdgeTrack();
	}
}

void SdrEdgeObj::DisconnectFromNode(bool bTail1)
{
	SdrObjConnection& rCon=GetConnection(bTail1);

	if(rCon.mpConnectedSdrObject) 
	{
		EndListening(*rCon.mpConnectedSdrObject);
		rCon.mpConnectedSdrObject = 0;
	}
}

SdrObject* SdrEdgeObj::GetConnectedNode(bool bTail1) const
{
	SdrObject* pObj = GetConnection(bTail1).mpConnectedSdrObject;
	
	if(pObj && (pObj->getSdrPageFromSdrObject() != getSdrPageFromSdrObject() || !pObj->IsObjectInserted())) 
	{
		pObj = 0;
	}
	
	return pObj;
}

bool SdrEdgeObj::CheckNodeConnection(bool bTail1) const
{
    bool bRet(false);
    const SdrObjConnection& rCon = GetConnection(bTail1);
    const sal_uInt32 nPtAnz(maEdgeTrack.count());
    
    if(rCon.mpConnectedSdrObject && rCon.mpConnectedSdrObject->getSdrPageFromSdrObject() == getSdrPageFromSdrObject() && nPtAnz) 
    {
        const sdr::glue::GluePointProvider& rProvider = rCon.mpConnectedSdrObject->GetGluePointProvider();

        if(rProvider.hasUserGluePoints())
        {
            const sdr::glue::GluePointVector aGluePointVector(rProvider.getUserGluePointVector());
            const sal_uInt32 nConAnz(aGluePointVector.size());
            const sal_uInt32 nGesAnz(nConAnz + rProvider.getAutoGluePointCount());
            const basegfx::B2DPoint aTail(bTail1 ? maEdgeTrack.getB2DPoint(0) : maEdgeTrack.getB2DPoint(nPtAnz - 1));

            for(sal_uInt32 i(0); i < nGesAnz && !bRet; i++) 
            {
                if(i < nConAnz) 
                { 
                    // UserDefined or CustomShape
                    const sdr::glue::GluePoint* pCandidate = aGluePointVector[i];

                    if(pCandidate)
                    {
                        const basegfx::B2DPoint aGluePos(rCon.mpConnectedSdrObject->getSdrObjectTransformation() * pCandidate->getUnitPosition());

                        bRet = (aTail == aGluePos);
                    }
                    else
                    {
                        OSL_ENSURE(false, "Got sdr::glue::PointVector with emty entries (!)");
                    }
                } 
                else //if (i<nConAnz+4) 
                { 
                    // Vertex
                    const sdr::glue::GluePoint aPt(rProvider.getAutoGluePointByIndex(i - nConAnz));
                    const basegfx::B2DPoint aGluePos(rCon.mpConnectedSdrObject->getSdrObjectTransformation() * aPt.getUnitPosition());

                    bRet = (aTail == aGluePos);
                }
            }
        }
    }

    return bRet;
}

void SdrEdgeObj::ImpSetTailPoint(bool bTail1, const basegfx::B2DPoint& rPt)
{
	const sal_uInt32 nPtAnz(maEdgeTrack.count());
	
    if(nPtAnz)
    {
		if(!bTail1) 
        {
            maEdgeTrack.setB2DPoint(nPtAnz - 1, rPt);
        }
		else 
        {
            maEdgeTrack.setB2DPoint(0, rPt);
        }
        
        ImpRecalcEdgeTrack();
	    ActionChanged/*formallyinvalidateobjectrange*/();
	}
}

void SdrEdgeObj::ImpDirtyEdgeTrack()
{
	if ( !mbEdgeTrackUserDefined )
	{
		mbEdgeTrackDirty = true;
	}
	else
	{
		if ( !(getSdrModelFromSdrObject().isLocked()) )
		{
    		mbEdgeTrackDirty = true;
        }
	}
}

void SdrEdgeObj::ImpUndirtyEdgeTrack()
{
	if (mbEdgeTrackDirty )
	{
		if (getSdrModelFromSdrObject().isLocked()) 
		{
    		ImpRecalcEdgeTrack();
	    }
    }
}

void SdrEdgeObj::ImpRecalcEdgeTrack()
{
    // #120437# if bEdgeTrackUserDefined, do not recalculate. Also not when model locked
	if(mbEdgeTrackUserDefined || getSdrModelFromSdrObject().isLocked())
    {
		return;
    }

    static bool mbBoundRectCalculationRunning = false;

    if(mbBoundRectCalculationRunning)
	{
		// this object is involved into another ImpRecalcEdgeTrack() call
		// from another SdrEdgeObj. Do not calculate again to avoid loop.
		// Also, do not change mbEdgeTrackDirty so that it gets recalculated
		// later at the first non-looping call.
	}
    else if(getSdrModelFromSdrObject().isLocked())
	{
		// avoid re-layout during imports/API call sequences
		// #i45294# but calc EdgeTrack and secure properties there
		mbBoundRectCalculationRunning = true;
		maEdgeTrack = ImpCalcEdgeTrack(maCon1, maCon2, &maEdgeInfo);
		ImpSetAttrToEdgeInfo();
		mbEdgeTrackDirty = false;
		mbBoundRectCalculationRunning = false;
	}
	else
	{
		// To not run in a depth loop, use a coloring algorythm on
		// SdrEdgeObj BoundRect calculations
		mbBoundRectCalculationRunning = true;

        {   // use local scope to trigger locally
            const SdrObjectChangeBroadcaster aSdrObjectChangeBroadcaster(*this);

		    maEdgeTrack = ImpCalcEdgeTrack(maCon1, maCon2, &maEdgeInfo);
		    ImpSetEdgeInfoToAttr(); // Die Werte aus maEdgeInfo in den Pool kopieren
		    mbEdgeTrackDirty = false;

		    // Only redraw here, no object change
		    ActionChanged();
        }

		// #110649#
		mbBoundRectCalculationRunning = false;
	}
}

sal_uInt16 SdrEdgeObj::ImpCalcEscAngle(SdrObject* pObj, const basegfx::B2DPoint& rPt) const
{
	if (pObj==NULL) return SDRESC_ALL;
	const basegfx::B2DRange aR(sdr::legacy::GetSnapRange(*pObj));
	long dxl(basegfx::fround(rPt.getX() - aR.getMinX()));
	long dyo(basegfx::fround(rPt.getY() - aR.getMinY()));
	long dxr(basegfx::fround(aR.getMaxX() - rPt.getX()));
	long dyu(basegfx::fround(aR.getMaxY() - rPt.getY()));
	bool bxMitt=Abs(dxl-dxr)<2;
	bool byMitt=Abs(dyo-dyu)<2;
	long dx=Min(dxl,dxr);
	long dy=Min(dyo,dyu);
	bool bDiag=Abs(dx-dy)<2;
	if (bxMitt && byMitt) return SDRESC_ALL; // In der Mitte
	if (bDiag) {  // diagonal
		sal_uInt16 nRet=0;
		if (byMitt) nRet|=SDRESC_VERT;
		if (bxMitt) nRet|=SDRESC_HORZ;
		if (dxl<dxr) { // Links
			if (dyo<dyu) nRet|=sdr::glue::GluePoint::ESCAPE_DIRECTION_LEFT | sdr::glue::GluePoint::ESCAPE_DIRECTION_TOP;
			else nRet|=sdr::glue::GluePoint::ESCAPE_DIRECTION_LEFT | sdr::glue::GluePoint::ESCAPE_DIRECTION_BOTTOM;
		} else {       // Rechts
			if (dyo<dyu) nRet|=sdr::glue::GluePoint::ESCAPE_DIRECTION_RIGHT | sdr::glue::GluePoint::ESCAPE_DIRECTION_TOP;
			else nRet|=sdr::glue::GluePoint::ESCAPE_DIRECTION_RIGHT | sdr::glue::GluePoint::ESCAPE_DIRECTION_BOTTOM;
		}
		return nRet;
	}
	if (dx<dy) { // waagerecht
		if (bxMitt) return SDRESC_HORZ;
		if (dxl<dxr) return sdr::glue::GluePoint::ESCAPE_DIRECTION_LEFT;
		else return sdr::glue::GluePoint::ESCAPE_DIRECTION_RIGHT;
	} else {     // senkrecht
		if (byMitt) return SDRESC_VERT;
		if (dyo<dyu) return sdr::glue::GluePoint::ESCAPE_DIRECTION_TOP;
		else return sdr::glue::GluePoint::ESCAPE_DIRECTION_BOTTOM;
	}
}

XPolygon ImpCalcObjToCenter(const Point& rStPt, long nEscAngle, const Rectangle& rRect, const Point& rMeeting)
{
	XPolygon aXP;
	aXP.Insert(XPOLY_APPEND,rStPt,XPOLY_NORMAL);
	bool bRts=nEscAngle==0;
	bool bObn=nEscAngle==9000;
	bool bLks=nEscAngle==18000;
	bool bUnt=nEscAngle==27000;

	Point aP1(rStPt); // erstmal den Pflichtabstand
	if (bLks) aP1.X()=rRect.Left();
	if (bRts) aP1.X()=rRect.Right();
	if (bObn) aP1.Y()=rRect.Top();
	if (bUnt) aP1.Y()=rRect.Bottom();

	bool bFinish=false;
	if (!bFinish) {
		Point aP2(aP1); // Und nun den Pflichtabstand ggf. bis auf Meetinghoehe erweitern
		if (bLks && rMeeting.X()<=aP2.X()) aP2.X()=rMeeting.X();
		if (bRts && rMeeting.X()>=aP2.X()) aP2.X()=rMeeting.X();
		if (bObn && rMeeting.Y()<=aP2.Y()) aP2.Y()=rMeeting.Y();
		if (bUnt && rMeeting.Y()>=aP2.Y()) aP2.Y()=rMeeting.Y();
		aXP.Insert(XPOLY_APPEND,aP2,XPOLY_NORMAL);

		Point aP3(aP2);
		if ((bLks && rMeeting.X()>aP2.X()) || (bRts && rMeeting.X()<aP2.X())) { // Aussenrum
			if (rMeeting.Y()<aP2.Y()) {
				aP3.Y()=rRect.Top();
				if (rMeeting.Y()<aP3.Y()) aP3.Y()=rMeeting.Y();
			} else {
				aP3.Y()=rRect.Bottom();
				if (rMeeting.Y()>aP3.Y()) aP3.Y()=rMeeting.Y();
			}
			aXP.Insert(XPOLY_APPEND,aP3,XPOLY_NORMAL);
			if (aP3.Y()!=rMeeting.Y()) {
				aP3.X()=rMeeting.X();
				aXP.Insert(XPOLY_APPEND,aP3,XPOLY_NORMAL);
			}
		}
		if ((bObn && rMeeting.Y()>aP2.Y()) || (bUnt && rMeeting.Y()<aP2.Y())) { // Aussenrum
			if (rMeeting.X()<aP2.X()) {
				aP3.X()=rRect.Left();
				if (rMeeting.X()<aP3.X()) aP3.X()=rMeeting.X();
			} else {
				aP3.X()=rRect.Right();
				if (rMeeting.X()>aP3.X()) aP3.X()=rMeeting.X();
			}
			aXP.Insert(XPOLY_APPEND,aP3,XPOLY_NORMAL);
			if (aP3.X()!=rMeeting.X()) {
				aP3.Y()=rMeeting.Y();
				aXP.Insert(XPOLY_APPEND,aP3,XPOLY_NORMAL);
			}
		}
	}
#ifdef DBG_UTIL
	if (aXP.GetPointCount()>4) {
		DBG_ERROR("SdrEdgeObj::ImpCalcObjToCenter(): Polygon hat mehr als 4 Punkte!");
	}
#endif
	return aXP;
}

// predeclaration
basegfx::B2DPolygon ImpOldCalcEdgeTrack(
    const Point& rPt1, long nAngle1, const Rectangle& rBoundRect1, const Rectangle& rBewareRect1,
	const Point& rPt2, long nAngle2, const Rectangle& rBoundRect2, const Rectangle& rBewareRect2,
    SdrEdgeKind eKind,
	sal_uInt32* pnQuality, SdrEdgeInfoRec* pInfo);

basegfx::B2DPolygon SdrEdgeObj::ImpCalcEdgeTrack(
    SdrObjConnection& rCon1, 
    SdrObjConnection& rCon2, 
    SdrEdgeInfoRec* pInfo) const
{
    const sal_uInt32 nCount(maEdgeTrack.count());
    basegfx::B2DPolygon aRetval;

    if(nCount)
    {
	    // get old start/end points
		basegfx::B2DPoint aPt1(maEdgeTrack.getB2DPoint(0));
		basegfx::B2DPoint aPt2(maEdgeTrack.getB2DPoint(nCount - 1));
        const basegfx::B2DRange aBaseRange(aPt1, aPt2);
	    sdr::glue::GluePoint aGP1,aGP2;
	    sal_uInt16 nEsc1(SDRESC_ALL);
	    sal_uInt16 nEsc2(SDRESC_ALL);
	    basegfx::B2DRange aBoundRange1;
	    basegfx::B2DRange aBoundRange2;
	    basegfx::B2DRange aBewareRange1;
	    basegfx::B2DRange aBewareRange2;
	    SdrPage* pOwningPage = getSdrPageFromSdrObject();
	    bool bCon1(rCon1.mpConnectedSdrObject && rCon1.mpConnectedSdrObject->getSdrPageFromSdrObject() == pOwningPage);
	    bool bCon2(rCon2.mpConnectedSdrObject && rCon2.mpConnectedSdrObject->getSdrPageFromSdrObject() == pOwningPage);
	    const SfxItemSet& rSet = GetObjectItemSet();

	    if(bCon1) 
	    {
		    if(rCon1.mpConnectedSdrObject == this)
		    {
			    aBoundRange1 = aBaseRange;
		    }
		    else
		    {
			    aBoundRange1 = rCon1.mpConnectedSdrObject->getObjectRange(0);
		    }
	
		    const sal_Int32 nH(((SdrEdgeNode1HorzDistItem&)rSet.Get(SDRATTR_EDGENODE1HORZDIST)).GetValue());
		    const sal_Int32 nV(((SdrEdgeNode1VertDistItem&)rSet.Get(SDRATTR_EDGENODE1VERTDIST)).GetValue());

            aBewareRange1 = basegfx::B2DRange(
                aBoundRange1.getMinX() - nH, aBoundRange1.getMinY() - nV,
                aBoundRange1.getMaxX() + nH, aBoundRange1.getMaxY() + nV);
	    } 
	    else 
	    {
		    aBewareRange1 = aBoundRange1 = basegfx::B2DRange(aPt1);
	    }
	
	    if(bCon2) 
	    {
		    if(rCon2.mpConnectedSdrObject == this) 
		    {
			    aBoundRange2 = aBaseRange;
		    }
		    else
		    {
			    aBoundRange2 = rCon2.mpConnectedSdrObject->getObjectRange(0);
		    }

		    const sal_Int32 nH(((SdrEdgeNode2HorzDistItem&)rSet.Get(SDRATTR_EDGENODE2HORZDIST)).GetValue());
		    const sal_Int32 nV(((SdrEdgeNode2VertDistItem&)rSet.Get(SDRATTR_EDGENODE2VERTDIST)).GetValue());

            aBewareRange2 = basegfx::B2DRange(
                aBoundRange2.getMinX() - nH, aBoundRange2.getMinY() - nV,
                aBoundRange2.getMaxX() + nH, aBoundRange2.getMaxY() + nV);
	    } 
	    else 
	    {
		    aBewareRange2 = aBoundRange2 = basegfx::B2DRange(aPt2);
	    }
	
	    sal_uInt32 nBestQual=0xFFFFFFFF;
	    SdrEdgeInfoRec aBestInfo;
	    const bool bAuto1(bCon1 && rCon1.mbBestVertex);
	    const bool bAuto2(bCon2 && rCon2.mbBestVertex);

        if(bAuto1) 
        {
            rCon1.mbAutoVertex = true;
        }

	    if(bAuto2) 
        {
            rCon2.mbAutoVertex = true;
        }

	    sal_uInt16 nBestAuto1(0);
	    sal_uInt16 nBestAuto2(0);
	    sal_uInt16 nAnz1(bAuto1 ? 4 : 1);
	    sal_uInt16 nAnz2(bAuto2 ? 4 : 1);

        // prepare vars for old interface stuff
        const Point aPoint1(basegfx::fround(aPt1.getX()), basegfx::fround(aPt1.getY()));
        const Point aPoint2(basegfx::fround(aPt2.getX()), basegfx::fround(aPt2.getY()));
        const Rectangle aBoundRect1(
            basegfx::fround(aBoundRange1.getMinX()), basegfx::fround(aBoundRange1.getMinY()), 
            basegfx::fround(aBoundRange1.getMaxX()), basegfx::fround(aBoundRange1.getMaxY()));
        const Rectangle aBoundRect2(
            basegfx::fround(aBoundRange2.getMinX()), basegfx::fround(aBoundRange2.getMinY()), 
            basegfx::fround(aBoundRange2.getMaxX()), basegfx::fround(aBoundRange2.getMaxY()));
        const Rectangle aBewareRect1(
            basegfx::fround(aBewareRange1.getMinX()), basegfx::fround(aBewareRange1.getMinY()), 
            basegfx::fround(aBewareRange1.getMaxX()), basegfx::fround(aBewareRange1.getMaxY()));
        const Rectangle aBewareRect2(
            basegfx::fround(aBewareRange2.getMinX()), basegfx::fround(aBewareRange2.getMinY()), 
            basegfx::fround(aBewareRange2.getMaxX()), basegfx::fround(aBewareRange2.getMaxY()));

        for(sal_uInt16 nNum1(0); nNum1 < nAnz1; nNum1++) 
        {
		    if(bAuto1) 
            {
                rCon1.mnConnectorId = nNum1;
            }

		    if(bCon1 && rCon1.TakeGluePoint(aGP1)) 
            {
			    aPt1 = rCon1.mpConnectedSdrObject->getSdrObjectTransformation() * aGP1.getUnitPosition();
			    nEsc1 = aGP1.getEscapeDirections();

                if(sdr::glue::GluePoint::ESCAPE_DIRECTION_SMART == nEsc1) 
                {
                    nEsc1 = ImpCalcEscAngle(rCon1.mpConnectedSdrObject, aPt1);
                }
		    }

            for(sal_uInt16 nNum2(0); nNum2 < nAnz2; nNum2++) 
            {
			    if(bAuto2) 
                {
                    rCon2.mnConnectorId = nNum2;
                }

			    if(bCon2 && rCon2.TakeGluePoint(aGP2)) 
                {
				    aPt2 = rCon2.mpConnectedSdrObject->getSdrObjectTransformation() * aGP2.getUnitPosition();
				    nEsc2 = aGP2.getEscapeDirections();

                    if(sdr::glue::GluePoint::ESCAPE_DIRECTION_SMART == nEsc2) 
                    {
                        nEsc2 = ImpCalcEscAngle(rCon2.mpConnectedSdrObject, aPt2);
                    }
			    }

                for(long nA1(0); nA1 < 36000; nA1 += 9000) 
                {
				    const sal_uInt16 nE1(!nA1 ? sdr::glue::GluePoint::ESCAPE_DIRECTION_RIGHT : 9000 == nA1 ? sdr::glue::GluePoint::ESCAPE_DIRECTION_TOP : 18000 == nA1 ? sdr::glue::GluePoint::ESCAPE_DIRECTION_LEFT : 27000 == nA1 ? sdr::glue::GluePoint::ESCAPE_DIRECTION_BOTTOM : 0);
				    
                    for(long nA2(0); nA2 < 36000; nA2 += 9000) 
                    {
					    const sal_uInt16 nE2(!nA2 ? sdr::glue::GluePoint::ESCAPE_DIRECTION_RIGHT : 9000 == nA2 ? sdr::glue::GluePoint::ESCAPE_DIRECTION_TOP : 18000 == nA2 ? sdr::glue::GluePoint::ESCAPE_DIRECTION_LEFT : 27000 == nA2 ? sdr::glue::GluePoint::ESCAPE_DIRECTION_BOTTOM : 0);

                        if((nEsc1 & nE1) && (nEsc2 & nE2)) 
                        {
						    sal_uInt32 nQual(0);
						    SdrEdgeInfoRec aInfo;

                            if(pInfo) 
                            {
                                aInfo = *pInfo;
                            }

						    const basegfx::B2DPolygon aXP(
                                ImpOldCalcEdgeTrack(
                                    aPoint1, nA1, aBoundRect1, aBewareRect1,
                                    aPoint2, nA2, aBoundRect2, aBewareRect2,
                                    ((SdrEdgeKindItem&)(GetObjectItem(SDRATTR_EDGEKIND))).GetValue(),
                                    &nQual,
                                    &aInfo));

						    if(nQual < nBestQual) 
                            {
							    aRetval = aXP;
							    nBestQual = nQual;
							    aBestInfo = aInfo;
							    nBestAuto1 = nNum1;
							    nBestAuto2 = nNum2;
						    }
					    }
				    }
			    }
		    }
	    }

        if(bAuto1) 
        {
            rCon1.mnConnectorId = nBestAuto1;
        }

	    if(bAuto2) 
        {
            rCon2.mnConnectorId = nBestAuto2;
        }

	    if(pInfo) 
        {
            *pInfo = aBestInfo;
        }
	    
        return aRetval;
    }

    return aRetval;
}

basegfx::B2DPolygon ImpOldCalcEdgeTrack(
    const Point& rPt1, long nAngle1, const Rectangle& rBoundRect1, const Rectangle& rBewareRect1,
	const Point& rPt2, long nAngle2, const Rectangle& rBoundRect2, const Rectangle& rBewareRect2,
    SdrEdgeKind eKind,
	sal_uInt32* pnQuality, SdrEdgeInfoRec* pInfo)
{
	bool bRts1=nAngle1==0;
	bool bObn1=nAngle1==9000;
	bool bLks1=nAngle1==18000;
	bool bUnt1=nAngle1==27000;
	bool bHor1=bLks1 || bRts1;
	bool bVer1=bObn1 || bUnt1;
	bool bRts2=nAngle2==0;
	bool bObn2=nAngle2==9000;
	bool bLks2=nAngle2==18000;
	bool bUnt2=nAngle2==27000;
	bool bHor2=bLks2 || bRts2;
	bool bVer2=bObn2 || bUnt2;
	bool bInfo=pInfo!=NULL;
	if (bInfo) {
		pInfo->cOrthoForm=0;
		pInfo->nAngle1=nAngle1;
		pInfo->nAngle2=nAngle2;
		pInfo->nObj1Lines=1;
		pInfo->nObj2Lines=1;
		pInfo->nMiddleLine=0xFFFF;
	}
	Point aPt1(rPt1);
	Point aPt2(rPt2);
	Rectangle aBoundRect1 (rBoundRect1 );
	Rectangle aBoundRect2 (rBoundRect2 );
	Rectangle aBewareRect1(rBewareRect1);
	Rectangle aBewareRect2(rBewareRect2);
	Point aMeeting((aPt1.X()+aPt2.X()+1)/2,(aPt1.Y()+aPt2.Y()+1)/2);
	bool bMeetingXMid=true;
	bool bMeetingYMid=true;
	if (eKind==SDREDGE_ONELINE) {
		XPolygon aXP(2);
		aXP[0]=rPt1;
		aXP[1]=rPt2;
		if (pnQuality!=NULL) {
			*pnQuality=Abs(rPt1.X()-rPt2.X())+Abs(rPt1.Y()-rPt2.Y());
		}
		return aXP.getB2DPolygon();
	} else if (eKind==SDREDGE_THREELINES) {
		XPolygon aXP(4);
		aXP[0]=rPt1;
		aXP[1]=rPt1;
		aXP[2]=rPt2;
		aXP[3]=rPt2;
		if (bRts1) aXP[1].X()=aBewareRect1.Right();  //+=500;
		if (bObn1) aXP[1].Y()=aBewareRect1.Top();    //-=500;
		if (bLks1) aXP[1].X()=aBewareRect1.Left();   //-=500;
		if (bUnt1) aXP[1].Y()=aBewareRect1.Bottom(); //+=500;
		if (bRts2) aXP[2].X()=aBewareRect2.Right();  //+=500;
		if (bObn2) aXP[2].Y()=aBewareRect2.Top();    //-=500;
		if (bLks2) aXP[2].X()=aBewareRect2.Left();   //-=500;
		if (bUnt2) aXP[2].Y()=aBewareRect2.Bottom(); //+=500;
		if (pnQuality!=NULL) {
			long nQ=Abs(aXP[1].X()-aXP[0].X())+Abs(aXP[1].Y()-aXP[0].Y());
				nQ+=Abs(aXP[2].X()-aXP[1].X())+Abs(aXP[2].Y()-aXP[1].Y());
				nQ+=Abs(aXP[3].X()-aXP[2].X())+Abs(aXP[3].Y()-aXP[2].Y());
			*pnQuality=nQ;
		}
		if (bInfo) {
			pInfo->nObj1Lines=2;
			pInfo->nObj2Lines=2;
			if (bHor1) {
				aXP[1].X()+=pInfo->aObj1Line2.getX();
			} else {
				aXP[1].Y()+=pInfo->aObj1Line2.getY();
			}
			if (bHor2) {
				aXP[2].X()+=pInfo->aObj2Line2.getX();
			} else {
				aXP[2].Y()+=pInfo->aObj2Line2.getY();
			}
		}
		return aXP.getB2DPolygon();
	}
	sal_uInt16 nIntersections=0;
	bool bForceMeeting=false; // Muss die Linie durch den MeetingPoint laufen?
	{
		Point aC1(aBewareRect1.Center());
		Point aC2(aBewareRect2.Center());
		if (aBewareRect1.Left()<=aBewareRect2.Right() && aBewareRect1.Right()>=aBewareRect2.Left()) {
			// Ueberschneidung auf der X-Achse
			long n1=Max(aBewareRect1.Left(),aBewareRect2.Left());
			long n2=Min(aBewareRect1.Right(),aBewareRect2.Right());
			aMeeting.X()=(n1+n2+1)/2;
		} else {
			// Ansonsten den Mittelpunkt des Freiraums
			if (aC1.X()<aC2.X()) {
				aMeeting.X()=(aBewareRect1.Right()+aBewareRect2.Left()+1)/2;
			} else {
				aMeeting.X()=(aBewareRect1.Left()+aBewareRect2.Right()+1)/2;
			}
		}
		if (aBewareRect1.Top()<=aBewareRect2.Bottom() && aBewareRect1.Bottom()>=aBewareRect2.Top()) {
			// Ueberschneidung auf der Y-Achse
			long n1=Max(aBewareRect1.Top(),aBewareRect2.Top());
			long n2=Min(aBewareRect1.Bottom(),aBewareRect2.Bottom());
			aMeeting.Y()=(n1+n2+1)/2;
		} else {
			// Ansonsten den Mittelpunkt des Freiraums
			if (aC1.Y()<aC2.Y()) {
				aMeeting.Y()=(aBewareRect1.Bottom()+aBewareRect2.Top()+1)/2;
			} else {
				aMeeting.Y()=(aBewareRect1.Top()+aBewareRect2.Bottom()+1)/2;
			}
		}
		// Im Prinzip gibt es 3 zu unterscheidene Faelle:
		//   1. Beide in die selbe Richtung
		//   2. Beide in genau entgegengesetzte Richtungen
		//   3. Einer waagerecht und der andere senkrecht
		long nXMin=Min(aBewareRect1.Left(),aBewareRect2.Left());
		long nXMax=Max(aBewareRect1.Right(),aBewareRect2.Right());
		long nYMin=Min(aBewareRect1.Top(),aBewareRect2.Top());
		long nYMax=Max(aBewareRect1.Bottom(),aBewareRect2.Bottom());
		//bool bBoundOverlap=aBoundRect1.Right()>aBoundRect2.Left() && aBoundRect1.Left()<aBoundRect2.Right() &&
		//					   aBoundRect1.Bottom()>aBoundRect2.Top() && aBoundRect1.Top()<aBoundRect2.Bottom();
		bool bBewareOverlap=aBewareRect1.Right()>aBewareRect2.Left() && aBewareRect1.Left()<aBewareRect2.Right() &&
								aBewareRect1.Bottom()>aBewareRect2.Top() && aBewareRect1.Top()<aBewareRect2.Bottom();
		unsigned nMainCase=3;
		if (nAngle1==nAngle2) nMainCase=1;
		else if ((bHor1 && bHor2) || (bVer1 && bVer2)) nMainCase=2;
		if (nMainCase==1) { // Fall 1: Beide in eine Richtung moeglich.
			if (bVer1) aMeeting.X()=(aPt1.X()+aPt2.X()+1)/2; // ist hier besser, als der
			if (bHor1) aMeeting.Y()=(aPt1.Y()+aPt2.Y()+1)/2; // Mittelpunkt des Freiraums
			// bX1Ok bedeutet, dass die Vertikale, die aus Obj1 austritt, keinen Konflikt mit Obj2 bildet, ...
			bool bX1Ok=aPt1.X()<=aBewareRect2.Left() || aPt1.X()>=aBewareRect2.Right();
			bool bX2Ok=aPt2.X()<=aBewareRect1.Left() || aPt2.X()>=aBewareRect1.Right();
			bool bY1Ok=aPt1.Y()<=aBewareRect2.Top() || aPt1.Y()>=aBewareRect2.Bottom();
			bool bY2Ok=aPt2.Y()<=aBewareRect1.Top() || aPt2.Y()>=aBewareRect1.Bottom();
			if (bLks1 && (bY1Ok || aBewareRect1.Left()<aBewareRect2.Right()) && (bY2Ok || aBewareRect2.Left()<aBewareRect1.Right())) {
				aMeeting.X()=nXMin;
				bMeetingXMid=false;
			}
			if (bRts1 && (bY1Ok || aBewareRect1.Right()>aBewareRect2.Left()) && (bY2Ok || aBewareRect2.Right()>aBewareRect1.Left())) {
				aMeeting.X()=nXMax;
				bMeetingXMid=false;
			}
			if (bObn1 && (bX1Ok || aBewareRect1.Top()<aBewareRect2.Bottom()) && (bX2Ok || aBewareRect2.Top()<aBewareRect1.Bottom())) {
				aMeeting.Y()=nYMin;
				bMeetingYMid=false;
			}
			if (bUnt1 && (bX1Ok || aBewareRect1.Bottom()>aBewareRect2.Top()) && (bX2Ok || aBewareRect2.Bottom()>aBewareRect1.Top())) {
				aMeeting.Y()=nYMax;
				bMeetingYMid=false;
			}
		} else if (nMainCase==2) {
			// Fall 2:
			bForceMeeting=true;
			if (bHor1) { // beide waagerecht
                /* 9 Moeglichkeiten:                   � � �                    */
                /*   2.1 Gegenueber, Ueberschneidung   � � �                    */
                /*       nur auf der Y-Achse           � � �                    */
                /*   2.2, 2.3 Gegenueber, vertikal versetzt. � � �   � � �      */
                /*            Ueberschneidung weder auf der  � � �   � � �      */
                /*            X- noch auf der Y-Achse        � � �   � � �      */
                /*   2.4, 2.5 Untereinander,   � � �   � � �                    */
                /*            Ueberschneidung  � � �   � � �                    */
                /*            nur auf X-Achse  � � �   � � �                    */
                /*   2.6, 2.7 Gegeneinander, vertikal versetzt. � � �   � � �   */
                /*            Ueberschneidung weder auf der     � � �   � � �   */
                /*            X- noch auf der Y-Achse.          � � �   � � �   */
                /*   2.8 Gegeneinander.       � � �                             */
                /*       Ueberschneidung nur  � � �                             */
                /*       auf der Y-Achse.     � � �                             */
                /*   2.9 Die BewareRects der Objekte ueberschneiden             */
                /*       sich auf X- und Y-Achse.                               */
                /* Die Faelle gelten entsprechend umgesetzt auch fuer           */
                /* senkrechte Linienaustritte.                                  */
                /* Die Faelle 2.1-2.7 werden mit dem Default-Meeting ausreichend*/
                /* gut behandelt. Spezielle MeetingPoints werden hier also nur  */
                /* fuer 2.8 und 2.9 bestimmt.                                   */

				// Normalisierung. aR1 soll der nach rechts und
				// aR2 der nach links austretende sein.
				Rectangle aBewR1(bRts1 ? aBewareRect1 : aBewareRect2);
				Rectangle aBewR2(bRts1 ? aBewareRect2 : aBewareRect1);
				Rectangle aBndR1(bRts1 ? aBoundRect1 : aBoundRect2);
				Rectangle aBndR2(bRts1 ? aBoundRect2 : aBoundRect1);
				if (aBewR1.Bottom()>aBewR2.Top() && aBewR1.Top()<aBewR2.Bottom()) {
					// Ueberschneidung auf der Y-Achse. Faelle 2.1, 2.8, 2.9
					if (aBewR1.Right()>aBewR2.Left()) {
						// Faelle 2.8, 2.9
						// Fall 2.8 ist immer Aussenrumlauf (bDirect=false).
						// Fall 2.9 kann auch Direktverbindung sein (bei geringer
						// Ueberschneidung der BewareRects ohne Ueberschneidung der
						// Boundrects wenn die Linienaustritte sonst das BewareRect
						// des jeweils anderen Objekts verletzen wuerden.
						bool bCase29Direct=false;
						bool bCase29=aBewR1.Right()>aBewR2.Left();
						if (aBndR1.Right()<=aBndR2.Left()) { // Fall 2.9 und keine Boundrectueberschneidung
							if ((aPt1.Y()>aBewareRect2.Top() && aPt1.Y()<aBewareRect2.Bottom()) ||
								(aPt2.Y()>aBewareRect1.Top() && aPt2.Y()<aBewareRect1.Bottom())) {
							   bCase29Direct=true;
							}
						}
						if (!bCase29Direct) {
							bool bObenLang=Abs(nYMin-aMeeting.Y())<=Abs(nYMax-aMeeting.Y());
							if (bObenLang) {
								aMeeting.Y()=nYMin;
							} else {
								aMeeting.Y()=nYMax;
							}
							bMeetingYMid=false;
							if (bCase29) {
								// und nun noch dafuer sorgen, dass das
								// umzingelte Obj nicht durchquert wird
								if ((aBewR1.Center().Y()<aBewR2.Center().Y()) != bObenLang) {
									aMeeting.X()=aBewR2.Right();
								} else {
									aMeeting.X()=aBewR1.Left();
								}
								bMeetingXMid=false;
							}
						} else {
							// Direkte Verbindung (3-Linien Z-Verbindung), da
							// Verletzung der BewareRects unvermeidlich ist.
							// Via Dreisatz werden die BewareRects nun verkleinert.
							long nWant1=aBewR1.Right()-aBndR1.Right(); // Abstand bei Obj1
							long nWant2=aBndR2.Left()-aBewR2.Left();   // Abstand bei Obj2
							long nSpace=aBndR2.Left()-aBndR1.Right(); // verfuegbarer Platz
							long nGet1=BigMulDiv(nWant1,nSpace,nWant1+nWant2);
							long nGet2=nSpace-nGet1;
							if (bRts1) { // Normalisierung zurueckwandeln
								aBewareRect1.Right()+=nGet1-nWant1;
								aBewareRect2.Left()-=nGet2-nWant2;
							} else {
								aBewareRect2.Right()+=nGet1-nWant1;
								aBewareRect1.Left()-=nGet2-nWant2;
							}
							nIntersections++; // Qualitaet herabsetzen
						}
					}
				}
			} else if (bVer1) { // beide senkrecht
				Rectangle aBewR1(bUnt1 ? aBewareRect1 : aBewareRect2);
				Rectangle aBewR2(bUnt1 ? aBewareRect2 : aBewareRect1);
				Rectangle aBndR1(bUnt1 ? aBoundRect1 : aBoundRect2);
				Rectangle aBndR2(bUnt1 ? aBoundRect2 : aBoundRect1);
				if (aBewR1.Right()>aBewR2.Left() && aBewR1.Left()<aBewR2.Right()) {
					// Ueberschneidung auf der Y-Achse. Faelle 2.1, 2.8, 2.9
					if (aBewR1.Bottom()>aBewR2.Top()) {
						// Faelle 2.8, 2.9
						// Fall 2.8 ist immer Aussenrumlauf (bDirect=false).
						// Fall 2.9 kann auch Direktverbindung sein (bei geringer
						// Ueberschneidung der BewareRects ohne Ueberschneidung der
						// Boundrects wenn die Linienaustritte sonst das BewareRect
						// des jeweils anderen Objekts verletzen wuerden.
						bool bCase29Direct=false;
						bool bCase29=aBewR1.Bottom()>aBewR2.Top();
						if (aBndR1.Bottom()<=aBndR2.Top()) { // Fall 2.9 und keine Boundrectueberschneidung
							if ((aPt1.X()>aBewareRect2.Left() && aPt1.X()<aBewareRect2.Right()) ||
								(aPt2.X()>aBewareRect1.Left() && aPt2.X()<aBewareRect1.Right())) {
							   bCase29Direct=true;
							}
						}
						if (!bCase29Direct) {
							bool bLinksLang=Abs(nXMin-aMeeting.X())<=Abs(nXMax-aMeeting.X());
							if (bLinksLang) {
								aMeeting.X()=nXMin;
							} else {
								aMeeting.X()=nXMax;
							}
							bMeetingXMid=false;
							if (bCase29) {
								// und nun noch dafuer sorgen, dass das
								// umzingelte Obj nicht durchquert wird
								if ((aBewR1.Center().X()<aBewR2.Center().X()) != bLinksLang) {
									aMeeting.Y()=aBewR2.Bottom();
								} else {
									aMeeting.Y()=aBewR1.Top();
								}
								bMeetingYMid=false;
							}
						} else {
							// Direkte Verbindung (3-Linien Z-Verbindung), da
							// Verletzung der BewareRects unvermeidlich ist.
							// Via Dreisatz werden die BewareRects nun verkleinert.
							long nWant1=aBewR1.Bottom()-aBndR1.Bottom(); // Abstand bei Obj1
							long nWant2=aBndR2.Top()-aBewR2.Top();   // Abstand bei Obj2
							long nSpace=aBndR2.Top()-aBndR1.Bottom(); // verfuegbarer Platz
							long nGet1=BigMulDiv(nWant1,nSpace,nWant1+nWant2);
							long nGet2=nSpace-nGet1;
							if (bUnt1) { // Normalisierung zurueckwandeln
								aBewareRect1.Bottom()+=nGet1-nWant1;
								aBewareRect2.Top()-=nGet2-nWant2;
							} else {
								aBewareRect2.Bottom()+=nGet1-nWant1;
								aBewareRect1.Top()-=nGet2-nWant2;
							}
							nIntersections++; // Qualitaet herabsetzen
						}
					}
				}
			}
		} else if (nMainCase==3) { // Fall 3: Einer waagerecht und der andere senkrecht. Sehr viele Fallunterscheidungen
            /* Kleine Legende: � � � � � -> Ohne Ueberschneidung, maximal Beruehrung.                   */
            /*                 � � � � � -> Ueberschneidung                                             */
            /*                 � � � � � -> Selbe Hoehe                                                 */
            /*                 � � � � � -> Ueberschneidung                                             */
            /*                 � � � � � -> Ohne Ueberschneidung, maximal Beruehrung.                   */
            /* Linienaustritte links �, rechts �, oben � und unten �.                                   */
            /* Insgesamt sind 96 Konstellationen moeglich, wobei einige nicht einmal                    */
            /* eindeutig einem Fall und damit einer Behandlungsmethode zugeordnet werden                */
            /* koennen.                                                                                 */
            /* 3.1: Hierzu moegen alle Konstellationen zaehlen, die durch den                           */
            /*      Default-MeetingPoint zufriedenstellend abgedeckt sind (20+12).                      */
            /*   � � � � �    � � � � �   Diese 12  � � � � �    � � � � �    � � � � �    � � � � �    */
            /*   � � � � �    � � � � �   Konstel.  � � � � �    � � � � �    � � � � �    � � � � �    */
            /*   � � � � �    � � � � �   jedoch    � � � � �    � � � � �    � � � � �    � � � � �    */
            /*   � � � � �    � � � � �   nur zum   � � � � �    � � � � �    � � � � �    � � � � �    */
            /*   � � � � �    � � � � �   Teil:     � � � � �    � � � � �    � � � � �    � � � � �    */
            /*   Letztere 16 Faelle scheiden aus, sobald sich die Objekte offen                         */
            /*   gegenueberstehen (siehe Fall 3.2).                                                     */
            /* 3.2: Die Objekte stehen sich offen gegenueber und somit ist eine                         */
            /*      Verbindung mit lediglich 2 Linien moeglich (4+20).                                  */
            /*      Dieser Fall hat 1. Prioritaet.                                                      */
            /*   � � � � �   � � � � �   Diese 20  � � � � �   � � � � �   � � � � �   � � � � �        */
            /*   � � � � �   � � � � �   Konstel.  � � � � �   � � � � �   � � � � �   � � � � �        */
            /*   � � � � �   � � � � �   jedoch    � � � � �   � � � � �   � � � � �   � � � � �        */
            /*   � � � � �   � � � � �   nur zum   � � � � �   � � � � �   � � � � �   � � � � �        */
            /*   � � � � �   � � � � �   Teil:     � � � � �   � � � � �   � � � � �   � � � � �        */
            /* 3.3: Die Linienaustritte zeigen vom anderen Objekt weg bzw. hinter                       */
            /*      dessen Ruecken vorbei (52+4).                                                       */
            /*   � � � � �   � � � � �   � � � � �   � � � � �   Diese 4   � � � � �   � � � � �        */
            /*   � � � � �   � � � � �   � � � � �   � � � � �   Konstel.  � � � � �   � � � � �        */
            /*   � � � � �   � � � � �   � � � � �   � � � � �   jedoch    � � � � �   � � � � �        */
            /*   � � � � �   � � � � �   � � � � �   � � � � �   nur zum   � � � � �   � � � � �        */
            /*   � � � � �   � � � � �   � � � � �   � � � � �   Teil:     � � � � �   � � � � �        */

			// Fall 3.2
			Rectangle aTmpR1(aBewareRect1);
			Rectangle aTmpR2(aBewareRect2);
			if (bBewareOverlap) {
				// Ueberschneidung der BewareRects: BoundRects fuer Check auf Fall 3.2 verwenden.
				aTmpR1=aBoundRect1;
				aTmpR2=aBoundRect2;
			}
			if ((((bRts1 && aTmpR1.Right ()<=aPt2.X()) || (bLks1 && aTmpR1.Left()>=aPt2.X())) &&
				 ((bUnt2 && aTmpR2.Bottom()<=aPt1.Y()) || (bObn2 && aTmpR2.Top ()>=aPt1.Y()))) ||
				(((bRts2 && aTmpR2.Right ()<=aPt1.X()) || (bLks2 && aTmpR2.Left()>=aPt1.X())) &&
				 ((bUnt1 && aTmpR1.Bottom()<=aPt2.Y()) || (bObn1 && aTmpR1.Top ()>=aPt2.Y())))) {
				// Fall 3.2 trifft zu: Verbindung mit lediglich 2 Linien
				bForceMeeting=true;
				bMeetingXMid=false;
				bMeetingYMid=false;
				if (bHor1) {
					aMeeting.X()=aPt2.X();
					aMeeting.Y()=aPt1.Y();
				} else {
					aMeeting.X()=aPt1.X();
					aMeeting.Y()=aPt2.Y();
				}
				// Falls Ueberschneidung der BewareRects:
				aBewareRect1=aTmpR1;
				aBewareRect2=aTmpR2;
			} else if ((((bRts1 && aBewareRect1.Right ()>aBewareRect2.Left  ()) ||
						 (bLks1 && aBewareRect1.Left  ()<aBewareRect2.Right ())) &&
						((bUnt2 && aBewareRect2.Bottom()>aBewareRect1.Top   ()) ||
						 (bObn2 && aBewareRect2.Top   ()<aBewareRect1.Bottom()))) ||
					   (((bRts2 && aBewareRect2.Right ()>aBewareRect1.Left  ()) ||
						 (bLks2 && aBewareRect2.Left  ()<aBewareRect1.Right ())) &&
						((bUnt1 && aBewareRect1.Bottom()>aBewareRect2.Top   ()) ||
						 (bObn1 && aBewareRect1.Top   ()<aBewareRect2.Bottom())))) {
				// Fall 3.3
				bForceMeeting=true;
				if (bRts1 || bRts2) { aMeeting.X()=nXMax; bMeetingXMid=false; }
				if (bLks1 || bLks2) { aMeeting.X()=nXMin; bMeetingXMid=false; }
				if (bUnt1 || bUnt2) { aMeeting.Y()=nYMax; bMeetingYMid=false; }
				if (bObn1 || bObn2) { aMeeting.Y()=nYMin; bMeetingYMid=false; }
			}
		}
	}

	XPolygon aXP1(ImpCalcObjToCenter(aPt1,nAngle1,aBewareRect1,aMeeting));
	XPolygon aXP2(ImpCalcObjToCenter(aPt2,nAngle2,aBewareRect2,aMeeting));
	sal_uInt16 nXP1Anz=aXP1.GetPointCount();
	sal_uInt16 nXP2Anz=aXP2.GetPointCount();
	if (bInfo) {
		pInfo->nObj1Lines=nXP1Anz; if (nXP1Anz>1) pInfo->nObj1Lines--;
		pInfo->nObj2Lines=nXP2Anz; if (nXP2Anz>1) pInfo->nObj2Lines--;
	}
	Point aEP1(aXP1[nXP1Anz-1]);
	Point aEP2(aXP2[nXP2Anz-1]);
	bool bInsMeetingPoint=aEP1.X()!=aEP2.X() && aEP1.Y()!=aEP2.Y();
	bool bHorzE1=aEP1.Y()==aXP1[nXP1Anz-2].Y(); // letzte Linie von XP1 horizontal?
	bool bHorzE2=aEP2.Y()==aXP2[nXP2Anz-2].Y(); // letzte Linie von XP2 horizontal?
	if (aEP1==aEP2 && (bHorzE1 && bHorzE2 && aEP1.Y()==aEP2.Y()) || (!bHorzE1 && !bHorzE2 && aEP1.X()==aEP2.X())) {
		// Sonderbehandlung fuer 'I'-Verbinder
		nXP1Anz--; aXP1.Remove(nXP1Anz,1);
		nXP2Anz--; aXP2.Remove(nXP2Anz,1);
		bMeetingXMid=false;
		bMeetingYMid=false;
	}
	if (bInsMeetingPoint) {
		aXP1.Insert(XPOLY_APPEND,aMeeting,XPOLY_NORMAL);
		if (bInfo) {
			// Durch einfuegen des MeetingPoints kommen 2 weitere Linie hinzu.
			// Evtl. wird eine von diesen die Mittellinie.
			if (pInfo->nObj1Lines==pInfo->nObj2Lines) {
				pInfo->nObj1Lines++;
				pInfo->nObj2Lines++;
			} else {
				if (pInfo->nObj1Lines>pInfo->nObj2Lines) {
					pInfo->nObj2Lines++;
					pInfo->nMiddleLine=nXP1Anz-1;
				} else {
					pInfo->nObj1Lines++;
					pInfo->nMiddleLine=nXP1Anz;
				}
			}
		}
	} else if (bInfo && aEP1!=aEP2 && nXP1Anz+nXP2Anz>=4) {
		// Durch Verbinden der beiden Enden kommt eine weitere Linie hinzu.
		// Dies wird die Mittellinie.
		pInfo->nMiddleLine=nXP1Anz-1;
	}
	sal_uInt16 nNum=aXP2.GetPointCount();
	if (aXP1[nXP1Anz-1]==aXP2[nXP2Anz-1] && nXP1Anz>1 && nXP2Anz>1) nNum--;
	while (nNum>0) {
		nNum--;
		aXP1.Insert(XPOLY_APPEND,aXP2[nNum],XPOLY_NORMAL);
	}
	sal_uInt16 nPntAnz=aXP1.GetPointCount();
	char cForm=0;
	if (bInfo || pnQuality!=NULL) {
		cForm='?';
		if (nPntAnz==2) cForm='I';
		else if (nPntAnz==3) cForm='L';
		else if (nPntAnz==4) { // Z oder U
			if (nAngle1==nAngle2) cForm='U';
			else cForm='Z';
        } else if (nPntAnz==4) { /* �-�  �-�  */
            /* ...                 -�     -�  */
		} else if (nPntAnz==6) { // S oder C oder ...
			if (nAngle1!=nAngle2) {
				// Fuer Typ S hat Linie2 dieselbe Richtung wie Linie4.
				// Bei Typ C sind die beiden genau entgegengesetzt.
				Point aP1(aXP1[1]);
				Point aP2(aXP1[2]);
				Point aP3(aXP1[3]);
				Point aP4(aXP1[4]);
				if (aP1.Y()==aP2.Y()) { // beide Linien Horz
					if ((aP1.X()<aP2.X())==(aP3.X()<aP4.X())) cForm='S';
					else cForm='C';
				} else { // sonst beide Linien Vert
					if ((aP1.Y()<aP2.Y())==(aP3.Y()<aP4.Y())) cForm='S';
					else cForm='C';
				}
			} else cForm='4'; // sonst der 3. Fall mit 5 Linien
		} else cForm='?';  //
		// Weitere Formen:
		if (bInfo) {
			pInfo->cOrthoForm=cForm;
			if (cForm=='I' || cForm=='L' || cForm=='Z' || cForm=='U') {
				pInfo->nObj1Lines=1;
				pInfo->nObj2Lines=1;
				if (cForm=='Z' || cForm=='U') {
					pInfo->nMiddleLine=1;
				} else {
					pInfo->nMiddleLine=0xFFFF;
				}
			} else if (cForm=='S' || cForm=='C') {
				pInfo->nObj1Lines=2;
				pInfo->nObj2Lines=2;
				pInfo->nMiddleLine=2;
			}
		}
	}
	if (pnQuality!=NULL) {
		sal_uInt32 nQual=0;
		sal_uInt32 nQual0=nQual; // Ueberlaeufe vorbeugen
		bool bOverflow=false;
		Point aPt0(aXP1[0]);
		for (sal_uInt16 nPntNum=1; nPntNum<nPntAnz; nPntNum++) {
			Point aPt1b(aXP1[nPntNum]);
			nQual+=Abs(aPt1b.X()-aPt0.X())+Abs(aPt1b.Y()-aPt0.Y());
			if (nQual<nQual0) bOverflow=true;
			nQual0=nQual;
			aPt0=aPt1b;
		}

		sal_uInt16 nTmp=nPntAnz;
		if (cForm=='Z') {
			nTmp=2; // Z-Form hat gute Qualitaet (nTmp=2 statt 4)
			sal_uInt32 n1=Abs(aXP1[1].X()-aXP1[0].X())+Abs(aXP1[1].Y()-aXP1[0].Y());
			sal_uInt32 n2=Abs(aXP1[2].X()-aXP1[1].X())+Abs(aXP1[2].Y()-aXP1[1].Y());
			sal_uInt32 n3=Abs(aXP1[3].X()-aXP1[2].X())+Abs(aXP1[3].Y()-aXP1[2].Y());
			// fuer moeglichst gleichlange Linien sorgen
			sal_uInt32 nBesser=0;
			n1+=n3;
			n3=n2/4;
			if (n1>=n2) nBesser=6;
			else if (n1>=3*n3) nBesser=4;
			else if (n1>=2*n3) nBesser=2;
			if (aXP1[0].Y()!=aXP1[1].Y()) nBesser++; // Senkrechte Startlinie kriegt auch noch einen Pluspunkt (fuer H/V-Prio)
			if (nQual>nBesser) nQual-=nBesser; else nQual=0;
		}
		if (nTmp>=3) {
			nQual0=nQual;
			nQual+=(sal_uInt32)nTmp*0x01000000;
			if (nQual<nQual0 || nTmp>15) bOverflow=true;
		}
		if (nPntAnz>=2) { // Austrittswinkel nochmal pruefen
			Point aP1(aXP1[1]); aP1-=aXP1[0];
			Point aP2(aXP1[nPntAnz-2]); aP2-=aXP1[nPntAnz-1];
			long nAng1=0; if (aP1.X()<0) nAng1=18000; if (aP1.Y()>0) nAng1=27000;
			if (aP1.Y()<0) nAng1=9000; if (aP1.X()!=0 && aP1.Y()!=0) nAng1=1; // Schraeg!?!
			long nAng2=0; if (aP2.X()<0) nAng2=18000; if (aP2.Y()>0) nAng2=27000;
			if (aP2.Y()<0) nAng2=9000; if (aP2.X()!=0 && aP2.Y()!=0) nAng2=1; // Schraeg!?!
			if (nAng1!=nAngle1) nIntersections++;
			if (nAng2!=nAngle2) nIntersections++;
		}

		// Fuer den Qualitaetscheck wieder die Original-Rects verwenden und
		// gleichzeitig checken, ob eins fuer die Edge-Berechnung verkleinert
		// wurde (z.B. Fall 2.9)
		aBewareRect1=rBewareRect1;
		aBewareRect2=rBewareRect2;

		for (sal_uInt16 i=0; i<nPntAnz; i++) {
			Point aPt1b(aXP1[i]);
			bool b1=aPt1b.X()>aBewareRect1.Left() && aPt1b.X()<aBewareRect1.Right() &&
						aPt1b.Y()>aBewareRect1.Top() && aPt1b.Y()<aBewareRect1.Bottom();
			bool b2=aPt1b.X()>aBewareRect2.Left() && aPt1b.X()<aBewareRect2.Right() &&
						aPt1b.Y()>aBewareRect2.Top() && aPt1b.Y()<aBewareRect2.Bottom();
			sal_uInt16 nInt0=nIntersections;
			if (i==0 || i==nPntAnz-1) {
				if (b1 && b2) nIntersections++;
			} else {
				if (b1) nIntersections++;
				if (b2) nIntersections++;
			}
			// und nun noch auf Ueberschneidungen checken
			if (i>0 && nInt0==nIntersections) {
				if (aPt0.Y()==aPt1b.Y()) { // Horizontale Linie
					if (aPt0.Y()>aBewareRect1.Top() && aPt0.Y()<aBewareRect1.Bottom() &&
						((aPt0.X()<=aBewareRect1.Left() && aPt1b.X()>=aBewareRect1.Right()) ||
						 (aPt1b.X()<=aBewareRect1.Left() && aPt0.X()>=aBewareRect1.Right()))) nIntersections++;
					if (aPt0.Y()>aBewareRect2.Top() && aPt0.Y()<aBewareRect2.Bottom() &&
						((aPt0.X()<=aBewareRect2.Left() && aPt1b.X()>=aBewareRect2.Right()) ||
						 (aPt1b.X()<=aBewareRect2.Left() && aPt0.X()>=aBewareRect2.Right()))) nIntersections++;
				} else { // Vertikale Linie
					if (aPt0.X()>aBewareRect1.Left() && aPt0.X()<aBewareRect1.Right() &&
						((aPt0.Y()<=aBewareRect1.Top() && aPt1b.Y()>=aBewareRect1.Bottom()) ||
						 (aPt1b.Y()<=aBewareRect1.Top() && aPt0.Y()>=aBewareRect1.Bottom()))) nIntersections++;
					if (aPt0.X()>aBewareRect2.Left() && aPt0.X()<aBewareRect2.Right() &&
						((aPt0.Y()<=aBewareRect2.Top() && aPt1b.Y()>=aBewareRect2.Bottom()) ||
						 (aPt1b.Y()<=aBewareRect2.Top() && aPt0.Y()>=aBewareRect2.Bottom()))) nIntersections++;
				}
			}
			aPt0=aPt1b;
		}
		if (nPntAnz<=1) nIntersections++;
		nQual0=nQual;
		nQual+=(sal_uInt32)nIntersections*0x10000000;
		if (nQual<nQual0 || nIntersections>15) bOverflow=true;

		if (bOverflow || nQual==0xFFFFFFFF) nQual=0xFFFFFFFE;
		*pnQuality=nQual;
	}
	if (bInfo) { // nun die Linienversaetze auf aXP1 anwenden
		if (pInfo->nMiddleLine!=0xFFFF) {
			const sal_uInt16 nIdx = (sal_uInt16)pInfo->ImpGetPolyIdx(MIDDLELINE,aXP1.GetPointCount());
			if (pInfo->ImpIsHorzLine(MIDDLELINE,aXP1.GetPointCount())) {
				aXP1[nIdx].Y()+=pInfo->aMiddleLine.getY();
				aXP1[nIdx+1].Y()+=pInfo->aMiddleLine.getY();
			} else {
				aXP1[nIdx].X()+=pInfo->aMiddleLine.getX();
				aXP1[nIdx+1].X()+=pInfo->aMiddleLine.getX();
			}
		}
		if (pInfo->nObj1Lines>=2) {
			const sal_uInt16 nIdx = (sal_uInt16)pInfo->ImpGetPolyIdx(OBJ1LINE2,aXP1.GetPointCount());
			if (pInfo->ImpIsHorzLine(OBJ1LINE2,aXP1.GetPointCount())) {
				aXP1[nIdx].Y()+=pInfo->aObj1Line2.getY();
				aXP1[nIdx+1].Y()+=pInfo->aObj1Line2.getY();
			} else {
				aXP1[nIdx].X()+=pInfo->aObj1Line2.getX();
				aXP1[nIdx+1].X()+=pInfo->aObj1Line2.getX();
			}
		}
		if (pInfo->nObj1Lines>=3) {
			const sal_uInt16 nIdx = (sal_uInt16)pInfo->ImpGetPolyIdx(OBJ1LINE3,aXP1.GetPointCount());
			if (pInfo->ImpIsHorzLine(OBJ1LINE3,aXP1.GetPointCount())) {
				aXP1[nIdx].Y()+=pInfo->aObj1Line3.getY();
				aXP1[nIdx+1].Y()+=pInfo->aObj1Line3.getY();
			} else {
				aXP1[nIdx].X()+=pInfo->aObj1Line3.getX();
				aXP1[nIdx+1].X()+=pInfo->aObj1Line3.getX();
			}
		}
		if (pInfo->nObj2Lines>=2) {
			const sal_uInt16 nIdx = (sal_uInt16)pInfo->ImpGetPolyIdx(OBJ2LINE2,aXP1.GetPointCount());
			if (pInfo->ImpIsHorzLine(OBJ2LINE2,aXP1.GetPointCount())) {
				aXP1[nIdx].Y()+=pInfo->aObj2Line2.getY();
				aXP1[nIdx+1].Y()+=pInfo->aObj2Line2.getY();
			} else {
				aXP1[nIdx].X()+=pInfo->aObj2Line2.getX();
				aXP1[nIdx+1].X()+=pInfo->aObj2Line2.getX();
			}
		}
		if (pInfo->nObj2Lines>=3) {
			const sal_uInt16 nIdx = (sal_uInt16)pInfo->ImpGetPolyIdx(OBJ2LINE3,aXP1.GetPointCount());
			if (pInfo->ImpIsHorzLine(OBJ2LINE3,aXP1.GetPointCount())) {
				aXP1[nIdx].Y()+=pInfo->aObj2Line3.getY();
				aXP1[nIdx+1].Y()+=pInfo->aObj2Line3.getY();
			} else {
				aXP1[nIdx].X()+=pInfo->aObj2Line3.getX();
				aXP1[nIdx+1].X()+=pInfo->aObj2Line3.getX();
			}
		}
	}
	// Nun mache ich ggf. aus dem Verbinder eine Bezierkurve
	if (eKind==SDREDGE_BEZIER && nPntAnz>2) {
		Point* pPt1=&aXP1[0];
		Point* pPt2=&aXP1[1];
		Point* pPt3=&aXP1[nPntAnz-2];
		Point* pPt4=&aXP1[nPntAnz-1];
		long dx1=pPt2->X()-pPt1->X();
		long dy1=pPt2->Y()-pPt1->Y();
		long dx2=pPt3->X()-pPt4->X();
		long dy2=pPt3->Y()-pPt4->Y();
		if (cForm=='L') { // nPntAnz==3
			aXP1.SetFlags(1,XPOLY_CONTROL);
			Point aPt3(*pPt2);
			aXP1.Insert(2,aPt3,XPOLY_CONTROL);
			nPntAnz=aXP1.GetPointCount();
			pPt1=&aXP1[0];
			pPt2=&aXP1[1];
			pPt3=&aXP1[nPntAnz-2];
			pPt4=&aXP1[nPntAnz-1];
			pPt2->X()-=dx1/3;
			pPt2->Y()-=dy1/3;
			pPt3->X()-=dx2/3;
			pPt3->Y()-=dy2/3;
		} else if (nPntAnz>=4 && nPntAnz<=6) { // Z oder U oder ...
			// fuer Alle Anderen werden die Endpunkte der Ausgangslinien
			// erstmal zu Kontrollpunkten. Bei nPntAnz>4 ist also noch
			// Nacharbeit erforderlich!
			aXP1.SetFlags(1,XPOLY_CONTROL);
			aXP1.SetFlags(nPntAnz-2,XPOLY_CONTROL);
			// Distanz x1.5
			pPt2->X()+=dx1/2;
			pPt2->Y()+=dy1/2;
			pPt3->X()+=dx2/2;
			pPt3->Y()+=dy2/2;
			if (nPntAnz==5) {
				// Vor und hinter dem Mittelpunkt jeweils
				// noch einen Kontrollpunkt einfuegen
				Point aCenter(aXP1[2]);
				long dx1b=aCenter.X()-aXP1[1].X();
				long dy1b=aCenter.Y()-aXP1[1].Y();
				long dx2b=aCenter.X()-aXP1[3].X();
				long dy2b=aCenter.Y()-aXP1[3].Y();
				aXP1.Insert(2,aCenter,XPOLY_CONTROL);
				aXP1.SetFlags(3,XPOLY_SYMMTR);
				aXP1.Insert(4,aCenter,XPOLY_CONTROL);
				aXP1[2].X()-=dx1b/2;
				aXP1[2].Y()-=dy1b/2;
				aXP1[3].X()-=(dx1b+dx2b)/4;
				aXP1[3].Y()-=(dy1b+dy2b)/4;
				aXP1[4].X()-=dx2b/2;
				aXP1[4].Y()-=dy2b/2;
			}
			if (nPntAnz==6) {
				Point aPt1b(aXP1[2]);
				Point aPt2b(aXP1[3]);
				aXP1.Insert(2,aPt1b,XPOLY_CONTROL);
				aXP1.Insert(5,aPt2b,XPOLY_CONTROL);
				long dx=aPt1b.X()-aPt2b.X();
				long dy=aPt1b.Y()-aPt2b.Y();
				aXP1[3].X()-=dx/2;
				aXP1[3].Y()-=dy/2;
				aXP1.SetFlags(3,XPOLY_SYMMTR);
				//aXP1[4].X()+=dx/2;
				//aXP1[4].Y()+=dy/2;
				aXP1.Remove(4,1); // weil identisch mit aXP1[3]
			}
		}
	}
	return aXP1.getB2DPolygon();
}

/*
Nach einer einfachen Rechnung koennte es max. 64 unterschiedliche Verlaeufe mit
5 Linien, 32 mit 4 Linien, 16 mit 3, 8 mit 2 Linien und 4 mit 1 Linie geben=124.
Normalisiert auf 1. Austrittswinkel nach rechts bleiben dann noch 31.
Dann noch eine vertikale Spiegelung wegnormalisiert bleiben noch 16
characteristische Verlaufszuege mit 1-5 Linien:
Mit 1 Linie (Typ 'I'):  --
Mit 2 Linien (Typ 'L'): -�
Mit 3 Linien (Typ 'U'):  -�  (Typ 'Z'):  �-
						 -�             -�
Mit 4 Linien: 1 ist nicht plausibel, 3 ist=2 (90deg Drehung). Verbleibt 2,4
	 �-�  ڿ  ��  ڿ                               ڿ  �-�
	-�   -�   -�  -�                              -�    -�
Mit 5 Linien: nicht plausibel sind 1,2,4,5. 7 ist identisch mit 3 (Richtungsumkehr)
			  Bleibt also 3,6 und 8.              '4'  'S'  'C'
	   �    �             -�   �-  �-�                  �-
	 �-�  �-�  �-�   �-�   ��  �� -� � �-�         �-�  ��  �-�
	-�   -�   -� �  -� �-  -�  -�  --� � �        -� �  -�  � �
Insgesamt sind also 9 Grundtypen zu unterscheiden die den 400 Konstellationen
aus Objektposition und Austrittswinkeln zuzuordnen sind.
4 der 9 Grundtypen haben eine 'Mittellinie'. Die Anzahl der zu Objektabstaende
je Objekt variiert von 0-3:
	 Mi   O1   O2   Anmerkung
'I':  n   0    0
'L':  n   0    0
'U':  n  0-1  0-1
'Z':  j   0    0
4.1:  j   0    1    = U+1 bzw. 1+U
4.2:  n  0-2  0-2   = Z+1
'4':  j   0    2    = Z+2
'S':  j   1    1    = 1+Z+1
'C':  n  0-3  0-3   = 1+U+1
*/

void __EXPORT SdrEdgeObj::Notify(SfxBroadcaster& rBC, const SfxHint& rHint)
{
	const SfxSimpleHint* pSimple=dynamic_cast< const SfxSimpleHint* >( &rHint);
	sal_uInt32 nId=pSimple==0 ? 0 : pSimple->GetId();
	bool bDataChg=nId==SFX_HINT_DATACHANGED;
	bool bDying=nId==SFX_HINT_DYING;
	bool bObj1 = maCon1.mpConnectedSdrObject && maCon1.mpConnectedSdrObject == &rBC;
	bool bObj2 = maCon2.mpConnectedSdrObject && maCon2.mpConnectedSdrObject == &rBC;

    if (bDying && (bObj1 || bObj2)) {
		// #35605# Dying vorher abfangen, damit AttrObj nicht
		// wg. vermeintlicher Vorlagenaenderung rumbroadcastet
		if (bObj1) maCon1.mpConnectedSdrObject = 0;
		if (bObj2) maCon2.mpConnectedSdrObject = 0;
		return; // Und mehr braucht hier nicht getan werden.
	}
	if ( bObj1 || bObj2 )
	{
		mbEdgeTrackUserDefined = false;
	}
    SdrTextObj::Notify(rBC,rHint);
    static sal_uInt16 nNotifyingCount = 0;

    if(!nNotifyingCount) 
    { 
        // Hier nun auch ein VerriegelungsFlag
		nNotifyingCount++;
        const SdrBaseHint* pSdrHint = dynamic_cast< const SdrBaseHint* >(&rHint);

		if (bDataChg) { // StyleSheet geaendert
			ImpSetAttrToEdgeInfo(); // Werte bei Vorlagenaenderung vom Pool nach maEdgeInfo kopieren
		}
		SdrPage* pOwningPage = getSdrPageFromSdrObject();
		if (bDataChg                                ||
			(bObj1 && maCon1.mpConnectedSdrObject->getSdrPageFromSdrObject() == pOwningPage) ||
			(bObj2 && maCon2.mpConnectedSdrObject->getSdrPageFromSdrObject() == pOwningPage) ||
			(pSdrHint && pSdrHint->GetSdrHintKind()==HINT_OBJREMOVED))
		{
			// Broadcasting nur, wenn auf der selben Page
            const SdrObjectChangeBroadcaster aSdrObjectChangeBroadcaster(*this);

            ImpDirtyEdgeTrack();
			ActionChanged();
		}
		
        nNotifyingCount--;
	}
}

/** updates edges that are connected to the edges of this object
	as if the connected objects send a repaint broadcast
	#103122#
*/
void SdrEdgeObj::ReformatEdge()
{
	if(maCon1.mpConnectedSdrObject)
	{
		SfxSimpleHint aHint( SFX_HINT_DATACHANGED );
		Notify( *maCon1.mpConnectedSdrObject, aHint );
	}

	if(maCon2.mpConnectedSdrObject)
	{
		SfxSimpleHint aHint( SFX_HINT_DATACHANGED );
		Notify( *maCon2.mpConnectedSdrObject, aHint );
    }
}

void SdrEdgeObj::TakeObjNameSingul(XubString& rName) const
{
	rName=ImpGetResStr(STR_ObjNameSingulEDGE);

	String aName( GetName() );
	if(aName.Len())
	{
		rName += sal_Unicode(' ');
		rName += sal_Unicode('\'');
		rName += aName;
		rName += sal_Unicode('\'');
	}
}

void SdrEdgeObj::TakeObjNamePlural(XubString& rName) const
{
	rName=ImpGetResStr(STR_ObjNamePluralEDGE);
}

basegfx::B2DPolyPolygon SdrEdgeObj::TakeXorPoly() const
{
    return basegfx::B2DPolyPolygon(GetEdgeTrackPath());
}

void SdrEdgeObj::SetEdgeTrackPath(const basegfx::B2DPolygon& rPoly)
{
	if(rPoly.count() > 1)
	{
		maEdgeTrack = rPoly;
		mbEdgeTrackDirty = false;
		mbEdgeTrackUserDefined = true;

        // #i110629# also set aRect and maSnapeRect dependent from maEdgeTrack
        const basegfx::B2DRange aPolygonBounds(maEdgeTrack.getB2DRange());
        sdr::legacy::SetSnapRange(*this, aPolygonBounds);
	}
    else
	{
		mbEdgeTrackDirty = true;
		mbEdgeTrackUserDefined = false;
	}
}

basegfx::B2DPolygon SdrEdgeObj::GetEdgeTrackPath() const
{
	if(mbEdgeTrackDirty) 
	{
		const_cast< SdrEdgeObj* >(this)->ImpRecalcEdgeTrack();
	}

	return maEdgeTrack;
}

void SdrEdgeObj::AddToHdlList(SdrHdlList& rHdlList) const
{
	// due to old object re-use ol methods
	const sal_uInt32 nCount(impOldGetHdlCount());

	for(sal_uInt32 a(0); a < nCount; a++)
	{
		impOldGetHdl(rHdlList, a);
	}
}

sal_uInt32 SdrEdgeObj::impOldGetHdlCount() const
{
	const SdrEdgeKind eKind(((SdrEdgeKindItem&)(GetObjectItem(SDRATTR_EDGEKIND))).GetValue());
	const sal_uInt32 nPntAnz(maEdgeTrack.count());
	sal_uInt32 nHdlAnz(0);

	if(nPntAnz) 
	{
		nHdlAnz = 2;

		if((SDREDGE_ORTHOLINES == eKind || SDREDGE_BEZIER == eKind) &&  4 <= nPntAnz) 
		{
			const sal_uInt32 nO1(maEdgeInfo.nObj1Lines > 0 ? maEdgeInfo.nObj1Lines - 1 : 0);
			const sal_uInt32 nO2(maEdgeInfo.nObj2Lines > 0 ? maEdgeInfo.nObj2Lines - 1 : 0);
			const sal_uInt32 nM(maEdgeInfo.nMiddleLine != 0xFFFF ? 1 : 0);

            nHdlAnz += nO1 + nO2 + nM;
		} 
		else if(SDREDGE_THREELINES == eKind && 4 == nPntAnz) 
		{
			if(GetConnectedNode(true)) 
            {
				nHdlAnz++;
            }

			if(GetConnectedNode(false)) 
            {
				nHdlAnz++;
            }
		}
	}

	return nHdlAnz;
}

SdrHdl* SdrEdgeObj::impOldGetHdl(SdrHdlList& rHdlList, sal_uInt32 nHdlNum) const
{
	SdrHdl* pHdl = 0;
	const sal_uInt32 nPntAnz(maEdgeTrack.count());

    if(nPntAnz) 
    {
		if(!nHdlNum) 
        {
			pHdl = new ImpEdgeHdl(rHdlList, *this, HDL_POLY, maEdgeTrack.getB2DPoint(0));
			
            if(maCon1.mpConnectedSdrObject && maCon1.mbBestVertex) 
            {
                pHdl->Set1PixMore(true);
            }
		} 
        else if(1 == nHdlNum) 
        {
            pHdl = new ImpEdgeHdl(rHdlList, *this, HDL_POLY, maEdgeTrack.getB2DPoint(nPntAnz - 1));

            if(maCon2.mpConnectedSdrObject && maCon2.mbBestVertex) 
            {
                pHdl->Set1PixMore(true);
            }
		} 
        else 
        {
			const SdrEdgeKind eKind(((SdrEdgeKindItem&)(GetObjectItem(SDRATTR_EDGEKIND))).GetValue());
			
            if(SDREDGE_ORTHOLINES == eKind || SDREDGE_BEZIER == eKind) 
            {
				const sal_uInt32 nO1(maEdgeInfo.nObj1Lines > 0 ? maEdgeInfo.nObj1Lines - 1 : 0);
				const sal_uInt32 nO2(maEdgeInfo.nObj2Lines > 0 ? maEdgeInfo.nObj2Lines - 1 : 0);
				const sal_uInt32 nM(maEdgeInfo.nMiddleLine != 0xFFFF ? 1 : 0);
				sal_uInt32 nNum(nHdlNum - 2);
				sal_Int32 nPt(0);
				SdrEdgeLineCode aSdrEdgeLineCode(OBJ1LINE2);

                if(nNum < nO1) 
                {
					nPt = nNum + 1;
					
                    if(0 == nNum) 
                    {
                        aSdrEdgeLineCode = OBJ1LINE2;
                    }

					if(1 == nNum) 
                    {
                        aSdrEdgeLineCode = OBJ1LINE3;
                    }
				} 
                else 
                {
					nNum = nNum - nO1;

                    if(nNum < nO2) 
                    {
						nPt = nPntAnz - 3 - nNum;

                        if(0 == nNum) 
                        {
                            aSdrEdgeLineCode = OBJ2LINE2;
                        }

						if(1 == nNum) 
                        {
                            aSdrEdgeLineCode = OBJ2LINE3;
                        }
					} 
                    else 
                    {
						nNum = nNum - nO2;

                        if(nNum < nM) 
                        {
							nPt = maEdgeInfo.nMiddleLine;
							aSdrEdgeLineCode = MIDDLELINE;
						}
					}
				}

                if(nPt > 0) 
                {
                    ImpEdgeHdl* pImpEdgeHdl = new ImpEdgeHdl(rHdlList, *this, HDL_POLY, (maEdgeTrack.getB2DPoint(nPt) + maEdgeTrack.getB2DPoint(nPt + 1)) * 0.5);
                    pHdl = pImpEdgeHdl;
					pImpEdgeHdl->SetLineCode(aSdrEdgeLineCode);
				}
			} 
            else if(SDREDGE_THREELINES == eKind) 
            {
				sal_uInt32 nNum(nHdlNum);

                if(!GetConnectedNode(true)) 
                {
                    nNum++;
                }

                ImpEdgeHdl* pImpEdgeHdl = new ImpEdgeHdl(rHdlList, *this, HDL_POLY, maEdgeTrack.getB2DPoint(nNum - 1));
                pHdl = pImpEdgeHdl;
				
                if(2 == nNum) 
                {
                    pImpEdgeHdl->SetLineCode(OBJ1LINE2);
                }

				if(3 == nNum) 
                {
                    pImpEdgeHdl->SetLineCode(OBJ2LINE2);
                }
			}
		}
		
        if(pHdl) 
        {
			pHdl->SetPointNum(nHdlNum);
		}
	}

    return pHdl;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

bool SdrEdgeObj::hasSpecialDrag() const
{
	return true;
}

SdrObject* SdrEdgeObj::getFullDragClone() const
{
	// use Clone operator
	SdrEdgeObj* pRetval = static_cast< SdrEdgeObj* >(CloneSdrObject());

	// copy connections for clone, SdrEdgeObj::operator= does not do this
    pRetval->ConnectToNode(true, GetConnectedNode(true));
    pRetval->ConnectToNode(false, GetConnectedNode(false));

	return pRetval;
}

bool SdrEdgeObj::beginSpecialDrag(SdrDragStat& rDrag) const
{
	if(!rDrag.GetActiveHdl()) 
        return false;

    rDrag.SetEndDragChangesAttributes(true);

    if(rDrag.GetActiveHdl()->GetPointNum() < 2) 
    {
        rDrag.SetNoSnap(true);
	}

    return true;
}

bool SdrEdgeObj::applySpecialDrag(SdrDragStat& rDragStat)
{
    const SdrEdgeObj* pOriginalEdge = dynamic_cast< const SdrEdgeObj* >(rDragStat.GetActiveHdl()->GetObj());
    const bool bOriginalEdgeModified(pOriginalEdge == this);

	if(!bOriginalEdgeModified && pOriginalEdge)
	{
        // copy connections when clone is modified. This is needed because
		// as preparation to this modification the data from the original object
		// was copied to the clone using the operator=. As can be seen there,
		// that operator does not copy the connections (for good reason)
		ConnectToNode(true, pOriginalEdge->GetConnection(true).GetObject());
		ConnectToNode(false, pOriginalEdge->GetConnection(false).GetObject());
	}

    if(rDragStat.GetActiveHdl()->GetPointNum() < 2) 
    {
		// start or end point connector drag
        const bool bDragA(0 == rDragStat.GetActiveHdl()->GetPointNum());
        const basegfx::B2DPoint aPointNow(rDragStat.GetNow());
		SdrView& rSdrView = rDragStat.GetSdrViewFromSdrDragStat();

		if(rSdrView.GetSdrPageView())
        {
            SdrObjConnection* pDraggedOne(bDragA ? &maCon1 : &maCon2);

			// clear connection
			DisconnectFromNode(bDragA);

			// look for new connection
			FindConnector(aPointNow, rSdrView, *pDraggedOne, pOriginalEdge);

			if(pDraggedOne->mpConnectedSdrObject)
			{
				// if found, officially connect to it; FindConnector only
				// sets mpConnectedSdrObject hard
				SdrObject* pNewConnection = pDraggedOne->mpConnectedSdrObject;
				pDraggedOne->mpConnectedSdrObject = 0;
				ConnectToNode(bDragA, pNewConnection);
			}

            if(!bOriginalEdgeModified) 
            {
				// show IA helper, but only do this during IA, so not when the original 
				// Edge gets modified in the last call
			    rDragStat.GetSdrViewFromSdrDragStat().SetConnectMarker(*pDraggedOne);
            }
        }

		// change maEdgeTrack to modified position
        if(bDragA)
        {
            maEdgeTrack.setB2DPoint(0, aPointNow);
        }
        else
        {
            maEdgeTrack.setB2DPoint(maEdgeTrack.count() - 1, aPointNow);
        }

		// reset edge info's offsets, this is a end point drag
        maEdgeInfo.aObj1Line2 = maEdgeInfo.aObj1Line3 = maEdgeInfo.aObj2Line2 = maEdgeInfo.aObj2Line3 = maEdgeInfo.aMiddleLine = basegfx::B2DPoint();
	} 
    else 
    { 
		// control point connector drag
	    const ImpEdgeHdl* pEdgeHdl = dynamic_cast< const ImpEdgeHdl* >(rDragStat.GetActiveHdl());

        if(pEdgeHdl)
        {
		    const SdrEdgeLineCode eLineCode = pEdgeHdl->GetLineCode();
		    const basegfx::B2DPoint aDist(rDragStat.GetNow() - rDragStat.GetStart()); 
            sal_Int32 nDist(basegfx::fround(pEdgeHdl->IsHorzDrag() ? aDist.getX() : aDist.getY()));

            nDist += maEdgeInfo.ImpGetLineVersatz(eLineCode, maEdgeTrack.count());
		    maEdgeInfo.ImpSetLineVersatz(eLineCode, maEdgeTrack.count(), nDist);
        }
	}

	// force recalc EdgeTrack
	maEdgeTrack = ImpCalcEdgeTrack(maCon1, maCon2, &maEdgeInfo);
	mbEdgeTrackDirty = false;

	// save EdgeInfos and mark object as user modified
	ImpSetEdgeInfoToAttr();
	mbEdgeTrackUserDefined = false;

    if(bOriginalEdgeModified) 
    {
		// hide connect marker helper again when original gets changed.
		// This happens at the end of the interaction
		rDragStat.GetSdrViewFromSdrDragStat().HideConnectMarker();
	}

   	return true;
}

String SdrEdgeObj::getSpecialDragComment(const SdrDragStat& rDrag) const
{
    const bool bCreateComment(this == rDrag.GetSdrViewFromSdrDragStat().GetCreateObj());

    if(bCreateComment)
    {
        return String();
    }
    else
    {
	    XubString aStr;
	    TakeMarkedDescriptionString(STR_DragEdgeTail, aStr);
	 
        return aStr;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

basegfx::B2DPolygon SdrEdgeObj::ImplAddConnectorOverlay(SdrDragMethod& rDragMethod, bool bTail1, bool bTail2, bool bDetail) const
{
	basegfx::B2DPolygon aResult;

	if(bDetail) 
	{
		SdrObjConnection aMyCon1(maCon1);
		SdrObjConnection aMyCon2(maCon2);
		
		SdrEdgeInfoRec aInfo(maEdgeInfo);
		aResult = ImpCalcEdgeTrack(aMyCon1, aMyCon2, &aInfo);
	} 
	else 
	{
		basegfx::B2DPoint aPt1(maEdgeTrack.getB2DPoint(0));
		basegfx::B2DPoint aPt2(maEdgeTrack.getB2DPoint(maEdgeTrack.count() - 1));
		
		if (maCon1.mpConnectedSdrObject && (maCon1.mbBestConnection || maCon1.mbBestVertex)) 
		{
			aPt1 = sdr::legacy::GetSnapRange(*maCon1.mpConnectedSdrObject).getCenter();
		}
		
		if (maCon2.mpConnectedSdrObject && (maCon2.mbBestConnection || maCon2.mbBestVertex)) 
		{
			aPt2 = sdr::legacy::GetSnapRange(*maCon2.mpConnectedSdrObject).getCenter();
		}
		
		if (bTail1) 
		{
            aPt1 *= rDragMethod.getCurrentTransformation();
		}

		if (bTail2) 
		{
            aPt2 *= rDragMethod.getCurrentTransformation();
		}

		aResult.append(aPt1);
		aResult.append(aPt2);
	}

	return aResult;
}

bool SdrEdgeObj::BegCreate(SdrDragStat& rDragStat)
{
	rDragStat.SetNoSnap(true);
    maEdgeTrack.clear();
    maEdgeTrack.append(rDragStat.GetStart());
    maEdgeTrack.append(rDragStat.GetNow());

	SdrView& rSdrView = rDragStat.GetSdrViewFromSdrDragStat();

	if(rSdrView.GetSdrPageView()) 
	{
		FindConnector(rDragStat.GetStart(), rSdrView, maCon1, this);
		ConnectToNode(true, maCon1.mpConnectedSdrObject);
	}
	
	maEdgeTrack = ImpCalcEdgeTrack(maCon1, maCon2, &maEdgeInfo);

	return true;
}

bool SdrEdgeObj::MovCreate(SdrDragStat& rDragStat)
{
    maEdgeTrack.setB2DPoint(maEdgeTrack.count() - 1, rDragStat.GetNow());

    SdrView& rSdrView = rDragStat.GetSdrViewFromSdrDragStat();

	if(rSdrView.GetSdrPageView()) 
	{
		FindConnector(rDragStat.GetNow(), rSdrView, maCon2, this);
		rDragStat.GetSdrViewFromSdrDragStat().SetConnectMarker(maCon2);

	}

    ActionChanged();
	ConnectToNode(false, maCon2.mpConnectedSdrObject);
	maEdgeTrack = ImpCalcEdgeTrack(maCon1, maCon2, &maEdgeInfo);
	mbEdgeTrackDirty = false;
	
	return true;
}

bool SdrEdgeObj::EndCreate(SdrDragStat& rDragStat, SdrCreateCmd eCmd)
{
	const bool bOk(SDRCREATE_FORCEEND == eCmd || rDragStat.GetPointAnz() >= 2);

    if(bOk) 
    {
		ConnectToNode(true, maCon1.mpConnectedSdrObject);
		ConnectToNode(false, maCon2.mpConnectedSdrObject);
		rDragStat.GetSdrViewFromSdrDragStat().HideConnectMarker();
		ImpSetEdgeInfoToAttr(); // Die Werte aus maEdgeInfo in den Pool kopieren
	}

	ActionChanged();
	
	return bOk;
}

bool SdrEdgeObj::BckCreate(SdrDragStat& rDragStat)
{
	rDragStat.GetSdrViewFromSdrDragStat().HideConnectMarker();

	return false;
}

void SdrEdgeObj::BrkCreate(SdrDragStat& rDragStat)
{
	rDragStat.GetSdrViewFromSdrDragStat().HideConnectMarker();
}

basegfx::B2DPolyPolygon SdrEdgeObj::TakeCreatePoly(const SdrDragStat& /*rStatDrag*/) const
{
    return basegfx::B2DPolyPolygon(maEdgeTrack);
}

void SdrEdgeObj::FindConnector(
    const basegfx::B2DPoint& rPt, 
    const SdrView& rSdrView, 
    SdrObjConnection& rCon, 
    const SdrEdgeObj* pThis, 
    OutputDevice* pOut)
{
    rCon.ResetVars();

    if(rSdrView.GetSdrPageView())
    {
        if(!pOut) 
            pOut = rSdrView.GetFirstOutputDevice(); // GetWin(0);

        if (!pOut) 
            return;

        const Point aPt(basegfx::fround(rPt.getX()), basegfx::fround(rPt.getY()));
        SdrObjList* pOL = rSdrView.GetSdrPageView()->GetCurrentObjectList();
        const SetOfByte& rVisLayer = rSdrView.GetSdrPageView()->GetVisibleLayers();
        // Sensitiver Bereich der Konnektoren ist doppelt so gross wie die Handles:
        sal_uInt16 nMarkHdSiz = rSdrView.GetMarkHdlSizePixel();
        Size aHalfConSiz(nMarkHdSiz,nMarkHdSiz);
        aHalfConSiz=pOut->PixelToLogic(aHalfConSiz);
        Size aHalfCenterSiz(2*aHalfConSiz.Width(),2*aHalfConSiz.Height());
        Rectangle aMouseRect(aPt,aPt);
        aMouseRect.Left()  -=aHalfConSiz.Width();
        aMouseRect.Top()   -=aHalfConSiz.Height();
        aMouseRect.Right() +=aHalfConSiz.Width();
        aMouseRect.Bottom()+=aHalfConSiz.Height();
        sal_uInt16 nBoundHitTol=(sal_uInt16)aHalfConSiz.Width()/2; if (nBoundHitTol==0) nBoundHitTol=1;
        sal_uInt32 no=pOL->GetObjCount();
        bool bFnd=false;
        SdrObjConnection aTestCon;
        SdrObjConnection aBestCon;
        bool bTestBoundHit=false;
        //bool bBestBoundHit=false;

        while (no>0 && !bFnd) 
        {
            // Problem: Gruppenobjekt mit verschiedenen Layern liefert LayerID 0 !!!!
            no--;
            SdrObject* pObj = pOL->GetObj(no);
            if (rVisLayer.IsSet(pObj->GetLayer()) && pObj->IsVisible() &&      // only visible objects
                    (pThis==NULL || pObj != pThis))  // nicht an mich selbst connecten
            {
                Rectangle aObjBound(sdr::legacy::GetBoundRect(*pObj));
                if (aObjBound.IsOver(aMouseRect)) 
                {
                    aTestCon.ResetVars();
                    bTestBoundHit=false;
                    const bool bEdge(pObj->IsSdrEdgeObj()); // kein BestCon fuer Edge
                    // Die Userdefined Konnektoren haben absolute Prioritaet.
                    // Danach kommt Vertex, Corner und Mitte(Best) gleich priorisiert.
                    // Zum Schluss kommt noch ein HitTest aufs Obj.
                    const sdr::glue::GluePointProvider& rProvider = pObj->GetGluePointProvider();
                    const sdr::glue::GluePointVector aUserGluePointVector(rProvider.getUserGluePointVector());
                    const sal_uInt32 nUserGluePointCount(aUserGluePointVector.size());
                    const sal_uInt32 nAutoGluePointCount(rProvider.getAutoGluePointCount());
                    const sal_uInt32 nAllGluePointCount(nUserGluePointCount + nAutoGluePointCount + 1);
                    bool bUserFnd=false;
                    sal_uInt32 nBestDist=0xFFFFFFFF;

                    for (sal_uInt32 i=0; i<nAllGluePointCount; i++) 
                    {
                        const bool bUser(i < nUserGluePointCount);
                        const bool bVertex(i >= nUserGluePointCount && i < nUserGluePointCount + nAutoGluePointCount);
                        const bool bCenter(i + 1 == nAllGluePointCount);
                        bool bOk=false;
                        Point aConPos;
                        sal_uInt32 nConNum=i;

                        if(bUser) 
                        {
                            const sdr::glue::GluePoint* pCandidate = aUserGluePointVector[nConNum];

                            if(pCandidate)
                            {
                                const basegfx::B2DPoint aPoint(pObj->getSdrObjectTransformation() * pCandidate->getUnitPosition());
                                
                                aConPos = Point(basegfx::fround(aPoint.getX()), basegfx::fround(aPoint.getY()));
                                nConNum = pCandidate->getID();
                                bOk = true;
                            }
                            else
                            {
                                OSL_ENSURE(false, "Got sdr::glue::PointVector with empty entries (!)");
                            }
                        } 
                        else if (bVertex && !bUserFnd) 
                        {
                            nConNum = nConNum - nUserGluePointCount;

                            const sdr::glue::GluePoint aLocalPt(rProvider.getAutoGluePointByIndex(nConNum));
                            const basegfx::B2DPoint aPoint(pObj->getSdrObjectTransformation() * aLocalPt.getUnitPosition());

                            aConPos = Point(basegfx::fround(aPoint.getX()), basegfx::fround(aPoint.getY()));
                            bOk = true;
                        }
                        else if (bCenter && !bUserFnd && !bEdge)
                        {
                            // #109007#
                            // Suppress default connect at object center
                            if(!pThis || !pThis->GetSuppressDefaultConnect())
                            {
                                // Edges nicht!
                                nConNum=0;
                                aConPos=aObjBound.Center();
                                bOk=true;
                            }
                        }
                        if (bOk && aMouseRect.IsInside(aConPos)) 
                        {
                            if (bUser) bUserFnd=true;
                            bFnd=true;
                            sal_uInt32 nDist=(sal_uInt32)Abs(aConPos.X()-aPt.X())+(sal_uInt32)Abs(aConPos.Y()-aPt.Y());
                            
                            if (nDist<nBestDist) 
                            {
                                nBestDist = nDist;
                                aTestCon.mpConnectedSdrObject = pObj;
                                aTestCon.mnConnectorId = nConNum;
                                aTestCon.mbAutoVertex = bVertex;
                                aTestCon.mbBestConnection = false; // bCenter;
                                aTestCon.mbBestVertex = bCenter;
                            }
                        }
                    }
                    // Falls kein Konnektor getroffen wird nochmal
                    // HitTest versucht fuer BestConnector (=bCenter)
                    const basegfx::B2DPoint aHitTestPos(aPt.X(), aPt.Y());
                    if(!bFnd && 
                        !bEdge && 
                        SdrObjectPrimitiveHit(*pObj, aHitTestPos, nBoundHitTol, rSdrView, false, 0))
                    {
                        // #109007#
                        // Suppress default connect at object inside bound
                        if(!pThis || !pThis->GetSuppressDefaultConnect())
                        {
                            bFnd = true;
                            aTestCon.mpConnectedSdrObject = pObj;
                            aTestCon.mbBestConnection = true;
                        }
                    }
                    if (bFnd) 
                    {
                        Rectangle aMouseRect2(aPt,aPt);
                        aMouseRect.Left()  -=nBoundHitTol;
                        aMouseRect.Top()   -=nBoundHitTol;
                        aMouseRect.Right() +=nBoundHitTol;
                        aMouseRect.Bottom()+=nBoundHitTol;
                        bTestBoundHit=aObjBound.IsOver(aMouseRect2);
                    }
                }
            }
        }

        rCon=aTestCon;
    }
}

const basegfx::B2DHomMatrix& SdrEdgeObj::getSdrObjectTransformation() const
{
    return SdrTextObj::getSdrObjectTransformation();
}

void SdrEdgeObj::setSdrObjectTransformation(const basegfx::B2DHomMatrix& rTransformation)
{
    if(rTransformation != getSdrObjectTransformation())
    {
        // #54102# handle start and end point if not connected
        const bool bCon1(maCon1.mpConnectedSdrObject && maCon1.mpConnectedSdrObject->getSdrPageFromSdrObject() == getSdrPageFromSdrObject());
        const bool bCon2(maCon2.mpConnectedSdrObject && maCon2.mpConnectedSdrObject->getSdrPageFromSdrObject() == getSdrPageFromSdrObject());
        const sal_uInt32 nCount(maEdgeTrack.count());
        const bool bApplyTransform(nCount && (!bCon1 || !bCon2 || mbEdgeTrackUserDefined));
        basegfx::B2DHomMatrix aCompleteTransform;

        if(bApplyTransform)
        {
            // get old transform and invert
            aCompleteTransform = getSdrObjectTransformation();
            aCompleteTransform.invert();
        }

        // call parent, set new transform
        SdrTextObj::setSdrObjectTransformation(rTransformation);

        if(bApplyTransform)
        {
            // multiply current transform (after change) to get full change
            aCompleteTransform = getSdrObjectTransformation() * aCompleteTransform;

            if(mbEdgeTrackUserDefined)
            {
                // #120437# special handling when track is user defined
                maEdgeTrack.transform(aCompleteTransform);
            }
            else
            {
                bool bDirtyEdgeTrack(false);

                if(!bCon1)
                {
                    // transform first point
                    maEdgeTrack.setB2DPoint(0, aCompleteTransform * maEdgeTrack.getB2DPoint(0));
                    bDirtyEdgeTrack = true;
                }

                if(!bCon2)
                {
                    // transform last point
                    maEdgeTrack.setB2DPoint(nCount - 1, aCompleteTransform * maEdgeTrack.getB2DPoint(nCount - 1));
                    bDirtyEdgeTrack = true;
                }

                if(bDirtyEdgeTrack)
                {
                    ImpDirtyEdgeTrack();
                }
            }
        }

        // if resize is not from paste, forget user distances
        if(!IsPasteResize())
        {
            maEdgeInfo.aObj1Line2 = maEdgeInfo.aObj1Line3 = maEdgeInfo.aObj2Line2 = maEdgeInfo.aObj2Line3 = maEdgeInfo.aMiddleLine = basegfx::B2DPoint();
        }
    }
}

SdrObject* SdrEdgeObj::DoConvertToPolygonObject(bool bBezier, bool bAddText) const
{
	SdrObject* pRet = ImpConvertMakeObj(basegfx::B2DPolyPolygon(maEdgeTrack), bBezier);

    if(bAddText)
    {
    	pRet = ImpConvertAddText(pRet, bBezier);
    }

	return pRet;
}

sal_uInt32 SdrEdgeObj::GetSnapPointCount() const
{
	return 2;
}

basegfx::B2DPoint SdrEdgeObj::GetSnapPoint(sal_uInt32 i) const
{
	const_cast< SdrEdgeObj* >(this)->ImpUndirtyEdgeTrack();

	if(!i) 
	{
        return maEdgeTrack.getB2DPoint(0);
	}
	else
	{
    	const sal_uInt32 nAnz(maEdgeTrack.count());

        if(nAnz)
        {
            return maEdgeTrack.getB2DPoint(nAnz - 1);
        }

	}

	return basegfx::B2DPoint();
}

bool SdrEdgeObj::IsPolygonObject() const
{
	return false;
}

sal_uInt32 SdrEdgeObj::GetObjectPointCount() const
{
	return 0;
}

basegfx::B2DPoint SdrEdgeObj::GetObjectPoint(sal_uInt32 i) const
{
	const_cast< SdrEdgeObj* >(this)->ImpUndirtyEdgeTrack();

	if(!i) 
	{
		return maEdgeTrack.getB2DPoint(0);
	}
	else 
	{
    	const sal_uInt32 nAnz(maEdgeTrack.count());

        if(nAnz)
        {
    		return maEdgeTrack.getB2DPoint(nAnz - 1);
        }
    }

	return basegfx::B2DPoint();
}

void SdrEdgeObj::SetObjectPoint(const basegfx::B2DPoint& rPnt, sal_uInt32 i)
{
	ImpUndirtyEdgeTrack();

	if(!i) 
	{
		maEdgeTrack.setB2DPoint(0, rPnt);
	}
	else 
	{
    	const sal_uInt32 nAnz(maEdgeTrack.count());

        if(nAnz)
        {
    		maEdgeTrack.setB2DPoint(nAnz - 1, rPnt);
        }
        else
        {
            OSL_ENSURE(false, "SetObjectPoint out of bound (!)");
        }
    }
    
	SetEdgeTrackDirty();
	ActionChanged();
}

SdrEdgeObjGeoData::SdrEdgeObjGeoData()
{
    // default connector
    maEdgeTrack.append(basegfx::B2DPoint(0.0, 0.0));
    maEdgeTrack.append(basegfx::B2DPoint(0.0, 100.0));
}

SdrEdgeObjGeoData::~SdrEdgeObjGeoData()
{
}

SdrObjGeoData* SdrEdgeObj::NewGeoData() const
{
	return new SdrEdgeObjGeoData;
}

void SdrEdgeObj::SaveGeoData(SdrObjGeoData& rGeo) const
{
	SdrTextObj::SaveGeoData(rGeo);

    SdrEdgeObjGeoData& rEGeo = dynamic_cast< SdrEdgeObjGeoData& >(rGeo);
	rEGeo.maCon1 = maCon1;
	rEGeo.maCon2 = maCon2;
	rEGeo.maEdgeTrack = maEdgeTrack;
	rEGeo.mbEdgeTrackDirty = mbEdgeTrackDirty;
	rEGeo.mbEdgeTrackUserDefined = mbEdgeTrackUserDefined;
	rEGeo.maEdgeInfo = maEdgeInfo;
}

void SdrEdgeObj::RestGeoData(const SdrObjGeoData& rGeo)
{
	SdrTextObj::RestGeoData(rGeo);
	const SdrEdgeObjGeoData& rEGeo = dynamic_cast< const SdrEdgeObjGeoData& >(rGeo);

	if(maCon1.mpConnectedSdrObject != rEGeo.maCon1.mpConnectedSdrObject) 
	{
		if(maCon1.mpConnectedSdrObject) 
		{
			EndListening(*maCon1.mpConnectedSdrObject);
		}

		maCon1 = rEGeo.maCon1;

		if(maCon1.mpConnectedSdrObject) 
		{
			StartListening(*maCon1.mpConnectedSdrObject);
    	}
	}
	
	if(maCon2.mpConnectedSdrObject != rEGeo.maCon2.mpConnectedSdrObject) 
	{
		if(maCon2.mpConnectedSdrObject) 
		{
			EndListening(*maCon2.mpConnectedSdrObject);
		}

		maCon2 = rEGeo.maCon2;

		if(maCon2.mpConnectedSdrObject) 
		{
			StartListening(*maCon2.mpConnectedSdrObject);
	    }
	}
	
	maEdgeTrack = rEGeo.maEdgeTrack;
	mbEdgeTrackDirty = rEGeo.mbEdgeTrackDirty;
	mbEdgeTrackUserDefined = rEGeo.mbEdgeTrackUserDefined;
	maEdgeInfo = rEGeo.maEdgeInfo;
}

basegfx::B2DPoint SdrEdgeObj::GetTailPoint( bool bTail ) const
{
	if(maEdgeTrack.count())
	{
		if(bTail)
		{
			return maEdgeTrack.getB2DPoint(0);
		}
		else
		{
        	const sal_uInt32 nAnz(maEdgeTrack.count());

            if(nAnz)
            {
                return maEdgeTrack.getB2DPoint(nAnz - 1);
            }
		}
	}

	return basegfx::B2DPoint();
}

void SdrEdgeObj::SetTailPoint( bool bTail, const basegfx::B2DPoint& rPt )
{
	ImpSetTailPoint( bTail, rPt );
	SetChanged();
}

/** this method is used by the api to set a glue point for a connection
    nId == -1 :		The best default point is automaticly choosen
    0 <= nId <= 3 : One of the default points is choosen
    nId >= 4 :		A user defined glue point is choosen
*/
void SdrEdgeObj::setGluePointIndex(bool bTail, sal_Int32 nIndex /* = -1 */ )
{
    SdrObjConnection& rConn1 = GetConnection( bTail );
    rConn1.SetAutoVertex( nIndex >= 0 && nIndex <= 3 );
    rConn1.SetBestConnection( nIndex < 0 );
    rConn1.SetBestVertex( nIndex < 0 );

    if( nIndex > 3 )
    {
        nIndex -= 4; // The start api index and the implementation index is now both 0

        // for user defined glue points we have
        // to get the id for this index first
        const SdrObject* pCandidate = rConn1.GetObject();

        if(pCandidate)
        {
            const sdr::glue::GluePointProvider& rProvider = pCandidate->GetGluePointProvider();

            if(!rProvider.hasUserGluePoints() || !rProvider.findUserGluePointByID(nIndex))
            {
                return;
            }
        }
    }
    else if( nIndex < 0 )
    {
        nIndex = 0;
    }

    rConn1.SetConnectorId(nIndex);

    SetChanged();
    ImpRecalcEdgeTrack();
}

/** this method is used by the api to return a glue point id for a connection.
	See setGluePointId for possible return values */
sal_Int32 SdrEdgeObj::getGluePointIndex( bool bTail )
{
	SdrObjConnection& rConn1 = GetConnection( bTail );
	sal_Int32 nId = -1;
	if( !rConn1.IsBestConnection() )
	{
		nId = rConn1.GetConnectorId();
		if( !rConn1.IsAutoVertex() )
			nId += 3;		// SJ: the start api index is 0, whereas the implementation in svx starts from 1
	}
	return nId;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// for geometry access -> GetEdgeTrackPath()

//::basegfx::B2DPolygon SdrEdgeObj::getEdgeTrack() const
//{
//	if(mbEdgeTrackDirty)
//	{
//		const_cast< SdrEdgeObj* >(this)->ImpRecalcEdgeTrack();
//	}
//
//	return maEdgeTrack;
//}

//////////////////////////////////////////////////////////////////////////////
// eof
