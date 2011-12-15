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
#include "precompiled_svgio.hxx"

#include <svgio/svgreader/svgclippathnode.hxx>
#include <drawinglayer/primitive2d/transformprimitive2d.hxx>
#include <drawinglayer/primitive2d/transparenceprimitive2d.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include <drawinglayer/geometry/viewinformation2d.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace svgio
{
    namespace svgreader
    {
        SvgClipPathNode::SvgClipPathNode(
            SvgDocument& rDocument,
            SvgNode* pParent)
        :   SvgNode(SVGTokenClipPathNode, rDocument, pParent),
            maSvgStyleAttributes(*this),
            mpaTransform(0),
            maClipPathUnits(userSpaceOnUse)
        {
        }

        SvgClipPathNode::~SvgClipPathNode()
        {
            if(mpaTransform) delete mpaTransform;
        }

        const SvgStyleAttributes* SvgClipPathNode::getSvgStyleAttributes() const
        {
            static rtl::OUString aClassStr(rtl::OUString::createFromAscii("clip-path"));
            maSvgStyleAttributes.checkForCssStyle(aClassStr);

            return &maSvgStyleAttributes;
        }

        void SvgClipPathNode::parseAttribute(const rtl::OUString& rTokenName, SVGToken aSVGToken, const rtl::OUString& aContent)
        {
            // call parent
            SvgNode::parseAttribute(rTokenName, aSVGToken, aContent);

            // read style attributes
            maSvgStyleAttributes.parseStyleAttribute(rTokenName, aSVGToken, aContent);

            // parse own
            switch(aSVGToken)
            {
                case SVGTokenStyle:
                {
                    maSvgStyleAttributes.readStyle(aContent);
                    break;
                }
                case SVGTokenTransform:
                {
                    const basegfx::B2DHomMatrix aMatrix(readTransform(aContent, *this));

                    if(!aMatrix.isIdentity())
                    {
                        setTransform(&aMatrix);
                    }
                    break;
                }
                case SVGTokenClipPathUnits:
                {
                    if(aContent.getLength())
                    {
                        if(aContent.match(commonStrings::aStrUserSpaceOnUse, 0))
                        {
                            setClipPathUnits(userSpaceOnUse);
                        }
                        else if(aContent.match(commonStrings::aStrObjectBoundingBox, 0))
                        {
                            setClipPathUnits(objectBoundingBox);
                        }
                    }
                    break;
                }
            }
        }

        void SvgClipPathNode::decomposeSvgNode(drawinglayer::primitive2d::Primitive2DVector& rTarget, bool bReferenced) const
        {
            drawinglayer::primitive2d::Primitive2DVector aNewTarget;

            // decompose childs
            SvgNode::decomposeSvgNode(aNewTarget, bReferenced);

            if(!aNewTarget.empty())
            {
                if(getTransform())
                {
                    // create embedding group element with transformation
                    rTarget.push_back(
                        new drawinglayer::primitive2d::TransformPrimitive2D(
                            *getTransform(),
                            drawinglayer::primitive2d::Primitive2DVectorToPrimitive2DSequence(aNewTarget)));
                    aNewTarget.clear();
                }
                else
                {
                    // append to current target
                    rTarget.insert(rTarget.end(), aNewTarget.begin(), aNewTarget.end());
                }
            }
        }

        void SvgClipPathNode::apply(drawinglayer::primitive2d::Primitive2DVector& rTarget) const
        {
            if(rTarget.size())
            {
                drawinglayer::primitive2d::Primitive2DVector aClipTarget;

                // get clipPath definition as primitives
                decomposeSvgNode(aClipTarget, true);

                if(aClipTarget.size())
                {
                    // put content and clip definition to primitive sequence
                    const drawinglayer::primitive2d::Primitive2DSequence aContent(drawinglayer::primitive2d::Primitive2DVectorToPrimitive2DSequence(rTarget));
                    drawinglayer::primitive2d::Primitive2DSequence aClip(drawinglayer::primitive2d::Primitive2DVectorToPrimitive2DSequence(aClipTarget));

                    if(objectBoundingBox == getClipPathUnits())
                    {
                        // clip is object-relative, embed in content transformation
                        const basegfx::B2DRange aContentRange(
                            drawinglayer::primitive2d::getB2DRangeFromPrimitive2DSequence(
                                aContent,
                                drawinglayer::geometry::ViewInformation2D()));

                        const drawinglayer::primitive2d::Primitive2DReference xTransform(
                            new drawinglayer::primitive2d::TransformPrimitive2D(
                                basegfx::tools::createScaleTranslateB2DHomMatrix(
                                    aContentRange.getRange(),
                                    aContentRange.getMinimum()),
                                aClip));

                        aClip = drawinglayer::primitive2d::Primitive2DSequence(&xTransform, 1);
                    }

                    // redefine target. Use TransparencePrimitive2D with created clip
                    // geometry. Using the automatically set mbIsClipPathContent at 
                    // SvgStyleAttributes the clip definition is without fill, stroke, 
                    // and strokeWidth and forced to black, thus being 100% opaque
                    rTarget.clear();
                    rTarget.push_back(
                        new drawinglayer::primitive2d::TransparencePrimitive2D(
                            aContent,
                            aClip));
                }
                else
                {
                    // An empty clipping path will completely clip away the element that had 
                    // the �clip-path� property applied. (Svg spec)
                    rTarget.clear();
                }
            }
        }

    } // end of namespace svgreader
} // end of namespace svgio

//////////////////////////////////////////////////////////////////////////////
// eof
