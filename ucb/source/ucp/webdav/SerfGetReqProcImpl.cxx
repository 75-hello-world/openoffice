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
#include "precompiled_ucb.hxx"

#include <SerfGetReqProcImpl.hxx>

using namespace com::sun::star;

namespace http_dav_ucp
{

SerfGetReqProcImpl::SerfGetReqProcImpl( const char* inPath,
                                        const com::sun::star::uno::Reference< SerfInputStream > & xioInStrm )
    : SerfRequestProcessorImpl( inPath )
    , xInputStream( xioInStrm )
    , xOutputStream()
    , mpHeaderNames( 0 )
    , mpResource( 0 )
{
}

SerfGetReqProcImpl::SerfGetReqProcImpl( const char* inPath,
                                        const com::sun::star::uno::Reference< SerfInputStream > & xioInStrm,
                                        const std::vector< ::rtl::OUString > & inHeaderNames,
                                        DAVResource & ioResource )
    : SerfRequestProcessorImpl( inPath )
    , xInputStream( xioInStrm )
    , xOutputStream()
    , mpHeaderNames( &inHeaderNames )
    , mpResource( &ioResource )
{
}

SerfGetReqProcImpl::SerfGetReqProcImpl( const char* inPath,
                                        const com::sun::star::uno::Reference< com::sun::star::io::XOutputStream > & xioOutStrm )
    : SerfRequestProcessorImpl( inPath )
    , xInputStream()
    , xOutputStream( xioOutStrm )
    , mpHeaderNames( 0 )
    , mpResource( 0 )
{
}

SerfGetReqProcImpl::SerfGetReqProcImpl( const char* inPath,
                                        const com::sun::star::uno::Reference< com::sun::star::io::XOutputStream > & xioOutStrm,
                                        const std::vector< ::rtl::OUString > & inHeaderNames,
                                        DAVResource & ioResource )
    : SerfRequestProcessorImpl( inPath )
    , xInputStream()
    , xOutputStream( xioOutStrm )
    , mpHeaderNames( &inHeaderNames )
    , mpResource( &ioResource )
{
}

SerfGetReqProcImpl::~SerfGetReqProcImpl()
{
}

serf_bucket_t * SerfGetReqProcImpl::createSerfRequestBucket( serf_request_t * inSerfRequest )
{
    // create serf request
    serf_bucket_t *req_bkt = serf_request_bucket_request_create( inSerfRequest, 
                                                                 "GET",
                                                                 getPathStr(),
                                                                 0,
                                                                 serf_request_get_alloc( inSerfRequest ) );

    // TODO - correct headers
    // set request header fields
    serf_bucket_t* hdrs_bkt = serf_bucket_request_get_headers( req_bkt );
    serf_bucket_headers_setn( hdrs_bkt, "User-Agent", "www.openoffice.org/ucb/" );
    serf_bucket_headers_setn( hdrs_bkt, "Accept-Encoding", "gzip");

    return req_bkt;
}

namespace
{
    apr_status_t Serf_ProcessResponseHeader( void* inUserData,
                                             const char* inHeaderName,
                                             const char* inHeaderValue )
    {
        SerfGetReqProcImpl* pReqProcImpl = static_cast< SerfGetReqProcImpl* >( inUserData );
        pReqProcImpl->processSingleResponseHeader( inHeaderName,
                                                   inHeaderValue );

        return APR_SUCCESS;
    }
} // end of anonymous namespace

bool SerfGetReqProcImpl::processSerfResponseBucket( serf_request_t * /*inSerfRequest*/,
                                                    serf_bucket_t * inSerfResponseBucket,
                                                    apr_pool_t * /*inAprPool*/,
                                                    apr_status_t & outStatus )
{
    const char* data;
    apr_size_t len;

    while (1) {
        outStatus = serf_bucket_read(inSerfResponseBucket, 8096, &data, &len);
        if (SERF_BUCKET_READ_ERROR(outStatus))
        {
            return true;
        }

        if ( len > 0 )
        {
            if ( xInputStream.is() )
            {
                xInputStream->AddToStream( data, len );
            }
            else if ( xOutputStream.is() )
            {
                const uno::Sequence< sal_Int8 > aDataSeq( (sal_Int8 *)data, len );
                xOutputStream->writeBytes( aDataSeq );
            }
        }

        /* are we done yet? */
        if (APR_STATUS_IS_EOF(outStatus)) 
        {
            // read response header, if requested
            if ( mpHeaderNames != 0 && mpResource != 0 )
            {
                serf_bucket_t* SerfHeaderBucket = serf_bucket_response_get_headers( inSerfResponseBucket );
                if ( SerfHeaderBucket != 0 )
                {
                    serf_bucket_headers_do( SerfHeaderBucket, 
                                            Serf_ProcessResponseHeader,
                                            this );
                }
            }

            outStatus = APR_EOF;
            return true;
        }

        /* have we drained the response so far? */
        if ( APR_STATUS_IS_EAGAIN( outStatus ) )
        {
            return false;
        }
    }

    /* NOTREACHED */
    return true;
}

void SerfGetReqProcImpl::processSingleResponseHeader( const char* inHeaderName,
                                                      const char* inHeaderValue )
{
    rtl::OUString aHeaderName( rtl::OUString::createFromAscii( inHeaderName ) );

    bool bStoreHeaderField = false;

    if ( mpHeaderNames->size() == 0 )
    {
        // store all header fields
        bStoreHeaderField = true;
    }
    else
    {
        // store only header fields which are requested
        std::vector< ::rtl::OUString >::const_iterator it( mpHeaderNames->begin() );
        const std::vector< ::rtl::OUString >::const_iterator end( mpHeaderNames->end() );

        while ( it != end )
        {
            // header names are case insensitive
            if ( (*it).equalsIgnoreAsciiCase( aHeaderName ) )
            {
                bStoreHeaderField = true;
                break;
            }
            else
            {
                ++it;
            }
        }
    }

    if ( bStoreHeaderField )
    {
        DAVPropertyValue thePropertyValue;
        thePropertyValue.IsCaseSensitive = false;
        thePropertyValue.Name = aHeaderName;
        thePropertyValue.Value <<= rtl::OUString::createFromAscii( inHeaderValue );
        mpResource->properties.push_back( thePropertyValue );
    }
}

} // namespace http_dav_ucp
