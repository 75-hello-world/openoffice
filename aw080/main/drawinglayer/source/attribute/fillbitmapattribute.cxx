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
#include "precompiled_drawinglayer.hxx"

#include <drawinglayer/attribute/fillbitmapattribute.hxx>
#include <vcl/bitmapex.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
	namespace attribute
	{
		class ImpFillBitmapAttribute
		{
		public:
			// refcounter
			sal_uInt32								mnRefCount;

            // data definitions
			BitmapEx								maBitmapEx;
			basegfx::B2DPoint						maTopLeft;
			basegfx::B2DVector						maSize;

			// bitfield
			bool									mbTiling : 1;

			ImpFillBitmapAttribute(
                const BitmapEx& rBitmapEx, 
                const basegfx::B2DPoint& rTopLeft, 
                const basegfx::B2DVector& rSize, 
                bool bTiling)
			:	mnRefCount(0),
		    	maBitmapEx(rBitmapEx),
			    maTopLeft(rTopLeft),
			    maSize(rSize),
			    mbTiling(bTiling)
		    {
		    }

            bool operator==(const ImpFillBitmapAttribute& rCandidate) const
		    {
			    return (maBitmapEx == rCandidate.maBitmapEx
				    && maTopLeft == rCandidate.maTopLeft
				    && maSize == rCandidate.maSize
				    && mbTiling == rCandidate.mbTiling);
		    }

			// data read access
			const BitmapEx& getBitmapEx() const { return maBitmapEx; }
			const basegfx::B2DPoint& getTopLeft() const { return maTopLeft; }
			const basegfx::B2DVector& getSize() const { return maSize; }
			bool getTiling() const { return mbTiling; }

            static ImpFillBitmapAttribute* get_global_default()
            {
                static ImpFillBitmapAttribute* pDefault = 0;

                if(!pDefault)
                {
                    pDefault = new ImpFillBitmapAttribute(
                        BitmapEx(),
                        basegfx::B2DPoint(),
                        basegfx::B2DVector(),
                        false);

                    // never delete; start with RefCount 1, not 0
    			    pDefault->mnRefCount++;
                }

                return pDefault;
            }
		};

        FillBitmapAttribute::FillBitmapAttribute(
            const BitmapEx& rBitmapEx, 
            const basegfx::B2DPoint& rTopLeft, 
            const basegfx::B2DVector& rSize, 
            bool bTiling)
		:	mpFillBitmapAttribute(new ImpFillBitmapAttribute(
                rBitmapEx, rTopLeft, rSize, bTiling))
		{
		}

		FillBitmapAttribute::FillBitmapAttribute()
        :	mpFillBitmapAttribute(ImpFillBitmapAttribute::get_global_default())
		{
			mpFillBitmapAttribute->mnRefCount++;
		}

        FillBitmapAttribute::FillBitmapAttribute(const FillBitmapAttribute& rCandidate)
		:	mpFillBitmapAttribute(rCandidate.mpFillBitmapAttribute)
		{
			mpFillBitmapAttribute->mnRefCount++;
		}

		FillBitmapAttribute::~FillBitmapAttribute()
		{
			if(mpFillBitmapAttribute->mnRefCount)
			{
				mpFillBitmapAttribute->mnRefCount--;
			}
			else
			{
				delete mpFillBitmapAttribute;
			}
		}

        bool FillBitmapAttribute::isDefault() const
        {
            return mpFillBitmapAttribute == ImpFillBitmapAttribute::get_global_default();
        }

        FillBitmapAttribute& FillBitmapAttribute::operator=(const FillBitmapAttribute& rCandidate)
		{
			if(rCandidate.mpFillBitmapAttribute != mpFillBitmapAttribute)
			{
				if(mpFillBitmapAttribute->mnRefCount)
				{
					mpFillBitmapAttribute->mnRefCount--;
				}
				else
				{
					delete mpFillBitmapAttribute;
				}
				
				mpFillBitmapAttribute = rCandidate.mpFillBitmapAttribute;
				mpFillBitmapAttribute->mnRefCount++;
			}

			return *this;
		}

		bool FillBitmapAttribute::operator==(const FillBitmapAttribute& rCandidate) const
		{
			if(rCandidate.mpFillBitmapAttribute == mpFillBitmapAttribute)
			{
				return true;
			}

			if(rCandidate.isDefault() != isDefault())
			{
				return false;
			}

			return (*rCandidate.mpFillBitmapAttribute == *mpFillBitmapAttribute);
		}

        const BitmapEx& FillBitmapAttribute::getBitmapEx() const 
        { 
            return mpFillBitmapAttribute->getBitmapEx(); 
        }

        const basegfx::B2DPoint& FillBitmapAttribute::getTopLeft() const 
        { 
            return mpFillBitmapAttribute->getTopLeft(); 
        }

        const basegfx::B2DVector& FillBitmapAttribute::getSize() const 
        { 
            return mpFillBitmapAttribute->getSize(); 
        }

        bool FillBitmapAttribute::getTiling() const 
        { 
            return mpFillBitmapAttribute->getTiling(); 
        }

    } // end of namespace attribute
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////
// eof
