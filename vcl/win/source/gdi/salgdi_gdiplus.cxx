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
#include "precompiled_vcl.hxx"

#include <stdio.h>
#include <string.h>

#include <tools/svwin.h>
#include <tools/debug.hxx>

#include <win/wincomp.hxx>
#include <win/saldata.hxx>
#include <win/salgdi.h>

#ifndef min
#define min(a,b)	(((a) < (b)) ? (a) : (b))
#endif
#ifndef max
#define max(a,b)	(((a) > (b)) ? (a) : (b))
#endif

#if defined _MSC_VER
#pragma warning(push, 1)
#endif

#include <GdiPlus.h>
#include <GdiPlusEnums.h>
#include <GdiPlusColor.h>

#if defined _MSC_VER
#pragma warning(pop)
#endif

#include <basegfx/polygon/b2dpolygon.hxx>

// -----------------------------------------------------------------------

void impAddB2DPolygonToGDIPlusGraphicsPathReal(Gdiplus::GraphicsPath& rPath, const basegfx::B2DPolygon& rPolygon, bool bNoLineJoin)
{
    sal_uInt32 nCount(rPolygon.count());

    if(nCount)
    {
        const sal_uInt32 nEdgeCount(rPolygon.isClosed() ? nCount : nCount - 1);
        const bool bControls(rPolygon.areControlPointsUsed());
        basegfx::B2DPoint aCurr(rPolygon.getB2DPoint(0));
        Gdiplus::PointF aFCurr(Gdiplus::REAL(aCurr.getX()), Gdiplus::REAL(aCurr.getY()));

        for(sal_uInt32 a(0); a < nEdgeCount; a++)
        {
	        const sal_uInt32 nNextIndex((a + 1) % nCount);
	        const basegfx::B2DPoint aNext(rPolygon.getB2DPoint(nNextIndex));
	        const Gdiplus::PointF aFNext(Gdiplus::REAL(aNext.getX()), Gdiplus::REAL(aNext.getY()));

	        if(bControls && (rPolygon.isNextControlPointUsed(a) || rPolygon.isPrevControlPointUsed(nNextIndex)))
	        {
		        const basegfx::B2DPoint aCa(rPolygon.getNextControlPoint(a));
		        const basegfx::B2DPoint aCb(rPolygon.getPrevControlPoint(nNextIndex));

		        rPath.AddBezier(
			        aFCurr, 
			        Gdiplus::PointF(Gdiplus::REAL(aCa.getX()), Gdiplus::REAL(aCa.getY())),
			        Gdiplus::PointF(Gdiplus::REAL(aCb.getX()), Gdiplus::REAL(aCb.getY())),
			        aFNext);
	        }
	        else
	        {
		        rPath.AddLine(aFCurr, aFNext);
	        }

	        if(a + 1 < nEdgeCount)
	        {
		        aFCurr = aFNext;

			    if(bNoLineJoin)
			    {
				    rPath.StartFigure();
			    }
	        }
        }
    }
}

void impAddB2DPolygonToGDIPlusGraphicsPathInteger(Gdiplus::GraphicsPath& rPath, const basegfx::B2DPolygon& rPolygon, bool bNoLineJoin)
{
    sal_uInt32 nCount(rPolygon.count());

    if(nCount)
    {
        const sal_uInt32 nEdgeCount(rPolygon.isClosed() ? nCount : nCount - 1);
        const bool bControls(rPolygon.areControlPointsUsed());
        basegfx::B2DPoint aCurr(rPolygon.getB2DPoint(0));
        Gdiplus::Point aICurr(INT(aCurr.getX()), INT(aCurr.getY()));

        for(sal_uInt32 a(0); a < nEdgeCount; a++)
        {
	        const sal_uInt32 nNextIndex((a + 1) % nCount);
	        const basegfx::B2DPoint aNext(rPolygon.getB2DPoint(nNextIndex));
	        const Gdiplus::Point aINext(INT(aNext.getX()), INT(aNext.getY()));

	        if(bControls && (rPolygon.isNextControlPointUsed(a) || rPolygon.isPrevControlPointUsed(nNextIndex)))
	        {
		        const basegfx::B2DPoint aCa(rPolygon.getNextControlPoint(a));
		        const basegfx::B2DPoint aCb(rPolygon.getPrevControlPoint(nNextIndex));

		        rPath.AddBezier(
			        aICurr, 
			        Gdiplus::Point(INT(aCa.getX()), INT(aCa.getY())),
			        Gdiplus::Point(INT(aCb.getX()), INT(aCb.getY())),
			        aINext);
	        }
	        else
	        {
		        rPath.AddLine(aICurr, aINext);
	        }

	        if(a + 1 < nEdgeCount)
	        {
		        aICurr = aINext;

			    if(bNoLineJoin)
			    {
				    rPath.StartFigure();
			    }
	        }
        }
    }
}

bool WinSalGraphics::drawPolyPolygon( const ::basegfx::B2DPolyPolygon& rPolyPolygon, double fTransparency)
{
	const sal_uInt32 nCount(rPolyPolygon.count());

	if(mbBrush && nCount && (fTransparency >= 0.0 && fTransparency < 1.0))
	{
		Gdiplus::Graphics aGraphics(mhDC);
		const sal_uInt8 aTrans((sal_uInt8)255 - (sal_uInt8)basegfx::fround(fTransparency * 255.0));
		Gdiplus::Color aTestColor(aTrans, SALCOLOR_RED(maFillColor), SALCOLOR_GREEN(maFillColor), SALCOLOR_BLUE(maFillColor));
		Gdiplus::SolidBrush aTestBrush(aTestColor);
		Gdiplus::GraphicsPath aPath;

		for(sal_uInt32 a(0); a < nCount; a++)
		{
            if(0 != a)
            {
                aPath.StartFigure(); // #i101491# not needed for first run
            }

			impAddB2DPolygonToGDIPlusGraphicsPathReal(aPath, rPolyPolygon.getB2DPolygon(a), false);
            aPath.CloseFigure();
		}

        if(getAntiAliasB2DDraw())
        {
            aGraphics.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);
        }
        else
        {
    		aGraphics.SetSmoothingMode(Gdiplus::SmoothingModeNone);
        }

		aGraphics.FillPath(&aTestBrush, &aPath);
	}

 	return true;
}

bool WinSalGraphics::drawPolyLine( 
    const basegfx::B2DPolygon& rPolygon, 
    double fTransparency, 
    const basegfx::B2DVector& rLineWidths, 
    basegfx::B2DLineJoin eLineJoin,
    com::sun::star::drawing::LineCap eLineCap)
{
    const sal_uInt32 nCount(rPolygon.count());

	if(mbPen && nCount)
	{
		Gdiplus::Graphics aGraphics(mhDC);
		const sal_uInt8 aTrans = (sal_uInt8)basegfx::fround( 255 * (1.0 - fTransparency) );
		Gdiplus::Color aTestColor(aTrans, SALCOLOR_RED(maLineColor), SALCOLOR_GREEN(maLineColor), SALCOLOR_BLUE(maLineColor));
		Gdiplus::Pen aTestPen(aTestColor, Gdiplus::REAL(rLineWidths.getX()));
		Gdiplus::GraphicsPath aPath;
		bool bNoLineJoin(false);

		switch(eLineJoin)
		{
			default : // basegfx::B2DLINEJOIN_NONE :
			{
				if(basegfx::fTools::more(rLineWidths.getX(), 0.0))
				{
					bNoLineJoin = true;
				}
				break;
			}
			case basegfx::B2DLINEJOIN_BEVEL :
			{
				aTestPen.SetLineJoin(Gdiplus::LineJoinBevel);
				break;
			}
			case basegfx::B2DLINEJOIN_MIDDLE :
			case basegfx::B2DLINEJOIN_MITER :
			{
				const Gdiplus::REAL aMiterLimit(15.0);
				aTestPen.SetMiterLimit(aMiterLimit);
				aTestPen.SetLineJoin(Gdiplus::LineJoinMiter);
				break;
			}
			case basegfx::B2DLINEJOIN_ROUND :
			{
				aTestPen.SetLineJoin(Gdiplus::LineJoinRound);
				break;
			}
		}

        switch(eLineCap)
        {
            default: /*com::sun::star::drawing::LineCap_BUTT*/
            {
                // nothing to do
                break;
            }
            case com::sun::star::drawing::LineCap_ROUND:
            {
                aTestPen.SetStartCap(Gdiplus::LineCapRound);
                aTestPen.SetEndCap(Gdiplus::LineCapRound);
                break;
            }
            case com::sun::star::drawing::LineCap_SQUARE:
            {
                aTestPen.SetStartCap(Gdiplus::LineCapSquare);
                aTestPen.SetEndCap(Gdiplus::LineCapSquare);
                break;
            }
        }

		if(nCount > 250 && basegfx::fTools::more(rLineWidths.getX(), 1.5))
        {
    		impAddB2DPolygonToGDIPlusGraphicsPathInteger(aPath, rPolygon, bNoLineJoin);
        }
        else
        {
    		impAddB2DPolygonToGDIPlusGraphicsPathReal(aPath, rPolygon, bNoLineJoin);
        }

        if(rPolygon.isClosed() && !bNoLineJoin)
        {
            // #i101491# needed to create the correct line joins
            aPath.CloseFigure();
        }
		
        if(getAntiAliasB2DDraw())
        {
    		aGraphics.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);
        }
        else
        {
    		aGraphics.SetSmoothingMode(Gdiplus::SmoothingModeNone);
        }

		aGraphics.DrawPath(&aTestPen, &aPath);
	}

	return true;
}

// -----------------------------------------------------------------------
