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

#include <SerfRequestProcessor.hxx>
#include <SerfRequestProcessorImpl.hxx>
#include <SerfRequestProcessorImplFac.hxx>
#include <SerfCallbacks.hxx>
#include <SerfSession.hxx>

#include <apr_strings.h>

namespace http_dav_ucp
{

SerfRequestProcessor::SerfRequestProcessor( SerfSession& rSerfSession,
                                            const rtl::OUString & inPath )
    : mrSerfSession( rSerfSession )
    , mPathStr( 0 )
    , mDestPathStr( 0 )
    , mpProcImpl( 0 )
    , mbProcessingDone( false )
    , mpDAVException()
    , mnHTTPStatusCode( SC_NONE )
    , mHTTPStatusCodeText()
    , mRedirectLocation()
    , mbSetupSerfRequestCalled( false )
    , mbAcceptSerfResponseCalled( false )
    , mbHandleSerfResponseCalled( false )
{
    mPathStr = apr_pstrdup( mrSerfSession.getAprPool(), 
                            rtl::OUStringToOString( inPath, RTL_TEXTENCODING_UTF8 ) );
}

SerfRequestProcessor::~SerfRequestProcessor()
{
    delete mpProcImpl;
    delete mpDAVException;
}

void SerfRequestProcessor::prepareProcessor()
{
    delete mpDAVException;
    mpDAVException = 0;
    mnHTTPStatusCode = SC_NONE;
    mHTTPStatusCodeText = rtl::OUString();
    mRedirectLocation = rtl::OUString();

    mbSetupSerfRequestCalled = false;
    mbAcceptSerfResponseCalled = false;
    mbHandleSerfResponseCalled = false;
}

// PROPFIND - allprop & named
bool SerfRequestProcessor::processPropFind( const Depth inDepth,
                                            const std::vector< ::rtl::OUString > & inPropNames,
                                            std::vector< DAVResource > & ioResources,
                                            apr_status_t& outSerfStatus )
{
    mpProcImpl = createPropFindReqProcImpl( mPathStr,
                                            inDepth,
                                            inPropNames,
                                            ioResources );
    outSerfStatus = runProcessor();

    return outSerfStatus == APR_SUCCESS;
}

// PROPFIND - property names
bool SerfRequestProcessor::processPropFind( const Depth inDepth,
                                            std::vector< DAVResourceInfo > & ioResInfo,
                                            apr_status_t& outSerfStatus )
{
    mpProcImpl = createPropFindReqProcImpl( mPathStr,
                                            inDepth,
                                            ioResInfo );
    outSerfStatus = runProcessor();

    return outSerfStatus == APR_SUCCESS;
}

// PROPPATCH
bool SerfRequestProcessor::processPropPatch( const std::vector< ProppatchValue > & inProperties,
                                             apr_status_t& outSerfStatus )
{
    mpProcImpl = createPropPatchReqProcImpl( mPathStr,
                                             inProperties );
    outSerfStatus = runProcessor();

    return outSerfStatus == APR_SUCCESS;
}

// GET
bool SerfRequestProcessor::processGet( const com::sun::star::uno::Reference< SerfInputStream >& xioInStrm, 
                                       apr_status_t& outSerfStatus )
{
    mpProcImpl = createGetReqProcImpl( mPathStr,
                                       xioInStrm );
    outSerfStatus = runProcessor();

    return outSerfStatus == APR_SUCCESS;
}

// GET inclusive header fields
bool SerfRequestProcessor::processGet( const com::sun::star::uno::Reference< SerfInputStream >& xioInStrm, 
                                       const std::vector< ::rtl::OUString > & inHeaderNames,
                                       DAVResource & ioResource,
                                       apr_status_t& outSerfStatus )
{
    mpProcImpl = createGetReqProcImpl( mPathStr,
                                       xioInStrm,
                                       inHeaderNames,
                                       ioResource );
    outSerfStatus = runProcessor();

    return outSerfStatus == APR_SUCCESS;
}

// GET
bool SerfRequestProcessor::processGet( const com::sun::star::uno::Reference< com::sun::star::io::XOutputStream >& xioOutStrm, 
                                       apr_status_t& outSerfStatus )
{
    mpProcImpl = createGetReqProcImpl( mPathStr,
                                       xioOutStrm );
    outSerfStatus = runProcessor();

    return outSerfStatus == APR_SUCCESS;
}

// GET inclusive header fields
bool SerfRequestProcessor::processGet( const com::sun::star::uno::Reference< com::sun::star::io::XOutputStream >& xioOutStrm, 
                                       const std::vector< ::rtl::OUString > & inHeaderNames,
                                       DAVResource & ioResource,
                                       apr_status_t& outSerfStatus )
{
    mpProcImpl = createGetReqProcImpl( mPathStr,
                                       xioOutStrm,
                                       inHeaderNames,
                                       ioResource );
    outSerfStatus = runProcessor();

    return outSerfStatus == APR_SUCCESS;
}

// HEAD
bool SerfRequestProcessor::processHead( const std::vector< ::rtl::OUString > & inHeaderNames,
                                        DAVResource & ioResource,
                                        apr_status_t& outSerfStatus )
{
    mpProcImpl = createHeadReqProcImpl( mPathStr,
                                        inHeaderNames,
                                        ioResource );
    outSerfStatus = runProcessor();

    return outSerfStatus == APR_SUCCESS;
}

// PUT
bool SerfRequestProcessor::processPut( const char* inData,
                                       apr_size_t inDataLen,
                                       apr_status_t& outSerfStatus )
{
    mpProcImpl = createPutReqProcImpl( mPathStr,
                                       inData,
                                       inDataLen );
    outSerfStatus = runProcessor();

    return outSerfStatus == APR_SUCCESS;
}

// POST
bool SerfRequestProcessor::processPost( const char* inData,
                                        apr_size_t inDataLen,
                                        const rtl::OUString & inContentType,
                                        const rtl::OUString & inReferer,
                                        const com::sun::star::uno::Reference< SerfInputStream >& xioInStrm,
                                        apr_status_t& outSerfStatus )
{
    mContentType = apr_pstrdup( mrSerfSession.getAprPool(), 
                                rtl::OUStringToOString( inContentType, RTL_TEXTENCODING_UTF8 ) );
    mReferer = apr_pstrdup( mrSerfSession.getAprPool(), 
                                rtl::OUStringToOString( inReferer, RTL_TEXTENCODING_UTF8 ) );
    mpProcImpl = createPostReqProcImpl( mPathStr,
                                        inData,
                                        inDataLen,
                                        mContentType,
                                        mReferer,
                                        xioInStrm );
    outSerfStatus = runProcessor();

    return outSerfStatus == APR_SUCCESS;
}

// POST
bool SerfRequestProcessor::processPost( const char* inData,
                                        apr_size_t inDataLen,
                                        const rtl::OUString & inContentType,
                                        const rtl::OUString & inReferer,
                                        const com::sun::star::uno::Reference< com::sun::star::io::XOutputStream >& xioOutStrm,
                                        apr_status_t& outSerfStatus )
{
    mContentType = apr_pstrdup( mrSerfSession.getAprPool(), 
                                rtl::OUStringToOString( inContentType, RTL_TEXTENCODING_UTF8 ) );
    mReferer = apr_pstrdup( mrSerfSession.getAprPool(), 
                                rtl::OUStringToOString( inReferer, RTL_TEXTENCODING_UTF8 ) );
    mpProcImpl = createPostReqProcImpl( mPathStr,
                                        inData,
                                        inDataLen,
                                        mContentType,
                                        mReferer,
                                        xioOutStrm );
    outSerfStatus = runProcessor();

    return outSerfStatus == APR_SUCCESS;
}

// DELETE
bool SerfRequestProcessor::processDelete( apr_status_t& outSerfStatus )
{
    mpProcImpl = createDeleteReqProcImpl( mPathStr );
    outSerfStatus = runProcessor();

    return outSerfStatus == APR_SUCCESS;
}

// MKCOL
bool SerfRequestProcessor::processMkCol( apr_status_t& outSerfStatus )
{
    mpProcImpl = createMkColReqProcImpl( mPathStr );
    outSerfStatus = runProcessor();

    return outSerfStatus == APR_SUCCESS;
}

// COPY
bool SerfRequestProcessor::processCopy( const rtl::OUString & inDestinationPath,
                                        const bool inOverwrite,
                                        apr_status_t& outSerfStatus )
{
    mDestPathStr = apr_pstrdup( mrSerfSession.getAprPool(), 
                                rtl::OUStringToOString( inDestinationPath, RTL_TEXTENCODING_UTF8 ) );
    mpProcImpl = createCopyReqProcImpl( mPathStr,
                                        mDestPathStr,
                                        inOverwrite );
    outSerfStatus = runProcessor();

    return outSerfStatus == APR_SUCCESS;
}

// MOVE
bool SerfRequestProcessor::processMove( const rtl::OUString & inDestinationPath,
                                        const bool inOverwrite,
                                        apr_status_t& outSerfStatus )
{
    mDestPathStr = apr_pstrdup( mrSerfSession.getAprPool(), 
                                rtl::OUStringToOString( inDestinationPath, RTL_TEXTENCODING_UTF8 ) );
    mpProcImpl = createMoveReqProcImpl( mPathStr,
                                        mDestPathStr,
                                        inOverwrite );
    outSerfStatus = runProcessor();

    return outSerfStatus == APR_SUCCESS;
}

apr_status_t SerfRequestProcessor::runProcessor()
{
    prepareProcessor();

    // create serf request
    serf_connection_request_create( mrSerfSession.getSerfConnection(), 
                                    Serf_SetupRequest,
                                    this );

    // perform serf request
    mbProcessingDone = false;
    apr_status_t status = APR_SUCCESS;
    serf_context_t* pSerfContext = mrSerfSession.getSerfContext();
    apr_pool_t* pAprPool = mrSerfSession.getAprPool();
    while ( true )
    {
        status = serf_context_run( pSerfContext, 
                                   SERF_DURATION_FOREVER, 
                                   pAprPool );
        if ( APR_STATUS_IS_TIMEUP( status ) )
        {
            continue;
        }
        if ( status != APR_SUCCESS ) 
        {
            break;
        }
        if ( mbProcessingDone )
        {
            break;
        }
    }

    postprocessProcessor( status );

    return status;
}

void SerfRequestProcessor::postprocessProcessor( const apr_status_t inStatus )
{
    if ( inStatus == APR_SUCCESS )
    {
        return;
    }

    // DEBUG INFO
    const char* SerfErrorStr = serf_error_string( inStatus );
    char AprErrorStr[256];
    apr_strerror( inStatus, AprErrorStr, sizeof( AprErrorStr ) );

    switch ( inStatus )
    {
    case APR_EGENERAL:
        // general error; <mnHTTPStatusCode> provides more information
        {
            // TODO: reactivate special handling copied from neon!?
            /*
            if ( mnHTTPStatusCode == SC_LOCKED )
            {
                if ( m_aSerfLockStore.findByUri(
                         makeAbsoluteURL( inPath ) ) == 0 )
                {
                    // locked by 3rd party
                    throw DAVException( DAVException::DAV_LOCKED );
                }
                else
                {
                    // locked by ourself
                    throw DAVException( DAVException::DAV_LOCKED_SELF );
                }
            }

            // Special handling for 400 and 412 status codes, which may indicate
            // that a lock previously obtained by us has been released meanwhile
            // by the server. Unfortunately, RFC is not clear at this point,
            // thus server implementations behave different...
            else if ( mnHTTPStatusCode == SC_BAD_REQUEST || mnHTTPStatusCode == SC_PRECONDITION_FAILED )
            {
                if ( removeExpiredLocktoken( makeAbsoluteURL( inPath ), rEnv ) )
                    throw DAVException( DAVException::DAV_LOCK_EXPIRED );
            }
            */
            switch ( mnHTTPStatusCode )
            {
            case SC_NONE:
                if ( !mbSetupSerfRequestCalled )
                {
                    mpDAVException = new DAVException( DAVException::DAV_HTTP_LOOKUP,
                                                       SerfUri::makeConnectionEndPointString( mrSerfSession.getHostName(),
                                                                                              mrSerfSession.getPort() ) );
                }
                else
                {
                    mpDAVException = new DAVException( DAVException::DAV_HTTP_ERROR, 
                                                       mHTTPStatusCodeText, 
                                                       mnHTTPStatusCode );
                }
                break;
            case SC_MOVED_PERMANENTLY:
            case SC_MOVED_TEMPORARILY:
            case SC_SEE_OTHER:
            case SC_TEMPORARY_REDIRECT:
                mpDAVException = new DAVException( DAVException::DAV_HTTP_REDIRECT, 
                                                   mRedirectLocation );
                break;
            default:
                mpDAVException = new DAVException( DAVException::DAV_HTTP_ERROR, 
                                                   mHTTPStatusCodeText, 
                                                   mnHTTPStatusCode );
                break;
            }
        }
        break;

    default:
        mpDAVException = new DAVException( DAVException::DAV_HTTP_ERROR );
        break;
    }

}

apr_status_t SerfRequestProcessor::provideSerfCredentials( char ** outUsername, 
                                         char ** outPassword,
                                         serf_request_t * inRequest, 
                                         int inCode, 
                                         const char *inAuthProtocol,
                                         const char *inRealm,
                                         apr_pool_t *inAprPool )
{
    return mrSerfSession.provideSerfCredentials( outUsername,
                                                 outPassword,
                                                 inRequest,
                                                 inCode,
                                                 inAuthProtocol,
                                                 inRealm,
                                                 inAprPool );
}

apr_status_t SerfRequestProcessor::setupSerfRequest( serf_request_t * inSerfRequest,
                                   serf_bucket_t ** outSerfRequestBucket,
                                   serf_response_acceptor_t * outSerfResponseAcceptor,
                                   void ** outSerfResponseAcceptorBaton,
                                   serf_response_handler_t * outSerfResponseHandler,
                                   void ** outSerfResponseHandlerBaton,
                                   apr_pool_t * /*inAprPool*/ )
{
    mbSetupSerfRequestCalled = true;
    *outSerfRequestBucket = mpProcImpl->createSerfRequestBucket( inSerfRequest );

    // apply callbacks for accepting response and handling response
    *outSerfResponseAcceptor = Serf_AcceptResponse;
    *outSerfResponseAcceptorBaton = this;
    *outSerfResponseHandler = Serf_HandleResponse;
    *outSerfResponseHandlerBaton = this;

    return APR_SUCCESS;
}

serf_bucket_t* SerfRequestProcessor::acceptSerfResponse( serf_request_t * inSerfRequest,
                                                         serf_bucket_t * inSerfStreamBucket,
                                                         apr_pool_t * inAprPool )
{
    mbAcceptSerfResponseCalled = true;
    return mrSerfSession.acceptSerfResponse( inSerfRequest,
                                             inSerfStreamBucket,
                                             inAprPool );
}

apr_status_t SerfRequestProcessor::handleSerfResponse( serf_request_t * inSerfRequest,
                                                       serf_bucket_t * inSerfResponseBucket,
                                                       apr_pool_t * inAprPool )
{
    mbHandleSerfResponseCalled = true;

    // some general response handling and error handling
    {
        if ( !inSerfResponseBucket ) 
        {
            /* A NULL response can come back if the request failed completely */
            mbProcessingDone = true;
            return APR_EGENERAL;
        }

        serf_status_line sl;
        apr_status_t status = serf_bucket_response_status( inSerfResponseBucket, &sl );
        if ( status ) 
        {
            mbProcessingDone = false; // allow another try in order to get a response
            return status;
        }
        // TODO - check, if response status code handling is correct
        mnHTTPStatusCode = ( sl.version != 0 && sl.code >= 0 )
                           ? static_cast< sal_uInt16 >( sl.code )
                           : SC_NONE;
        if ( sl.reason )
        {
            mHTTPStatusCodeText = ::rtl::OUString::createFromAscii( sl.reason );
        }
        if ( ( sl.version == 0 || sl.code < 0 ) || 
             mnHTTPStatusCode >= 300 )
        {
            if ( mnHTTPStatusCode == 301 ||
                 mnHTTPStatusCode == 302 ||
                 mnHTTPStatusCode == 303 ||
                 mnHTTPStatusCode == 307 )
            {
                // new location for certain redirections
                serf_bucket_t *headers = serf_bucket_response_get_headers( inSerfResponseBucket );
                const char* location = serf_bucket_headers_get( headers, "Location" );
                if ( location )
                {
                    mRedirectLocation = rtl::OUString::createFromAscii( location );
                }
                mbProcessingDone = true;
                return APR_EGENERAL;
            }
            else if ( mrSerfSession.isHeadRequestInProgress() &&
                      ( mnHTTPStatusCode == 401 || mnHTTPStatusCode == 407 ) )
            {
                // keep going as authentication is not required on HEAD request.
                // the response already contains header fields.
            }
            else
            {
                mbProcessingDone = true;
                return APR_EGENERAL;
            }
        }
    }

    // request specific processing of the response bucket
    apr_status_t status = APR_SUCCESS;
    mbProcessingDone = mpProcImpl->processSerfResponseBucket( inSerfRequest,
                                                              inSerfResponseBucket,
                                                              inAprPool,
                                                              status );

    return status;
}

} // namespace http_dav_ucp

