/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 *
 */

#include <memory>
#include <QApplication>
#include <QByteArray>
#include <QNetworkRequest>
#include <QTemporaryFile>
#include <QSslConfiguration>
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
#include <QtConcurrent/QtConcurrentRun>
#else
#include <QtConcurrentRun>
#endif
#include <QDateTime>

#include "netservice.h"
#include "common.h"

using std::unique_ptr;

// constructor
NetService::NetService (qint16  nettimeout, QString httpHeader, QObject *parent)
{
  acceptRequests = true;	
  timeout = nettimeout;
  header = httpHeader;
  cookie.truncate (0);
  Request.reserve (10);
  Mutex = new QMutex (QMutex::Recursive);
  if (parent != nullptr)
    setParent (parent);
}

// destructor
NetService::~NetService ()
{
  delete Mutex;
}

// add request in vector
bool
NetService::addRequest_core (QString url, void * replyBuffer)
{
  if (!acceptRequests)
    return false;
    	
  for (const auto request : Request)
    if (request->url == url)
      return false;

  for (const auto request : Request)
    if (request->actualUrl == url)
      return false;

  NetRequestClass *request = new (std::nothrow) NetRequestClass;
  if (request == nullptr)
    return false;

  request->actualUrl = request->url = url.trimmed ().simplified ();
  request->finished = 0;
  request->error = CG_ERR_OK;
  request->replyBytes = replyBuffer;
  Request += request;

#ifdef DEBUG
  qDebug () << objectName () << "Added:" << Request.size () << url;
#endif

  return true;
}

// check request's status
int
NetService::checkRequestStatus_core (QString url, int & error)
{
  url = url.trimmed ().simplified ();
  for (auto request : Request)
    if (request->url == url)
    {
      error = request->error;
      return request->finished;
    }

  for (auto request : Request)
    if (request->actualUrl == url)
    {
      error = request->error;
      return request->finished;
    }

  return 0;
}

// get reply buffer
void *
NetService::getReplyBuffer_core (QString url)
{
  url = url.trimmed ().simplified ();
  for (const auto request : Request)
  {
    if (request->url == url || request->actualUrl == url)
      return request->replyBytes;
  }

  return nullptr;
}

// set the status of a request
void
NetService::setRequestStatus_core (QString url, const int status, const int error)
{
  url = url.trimmed ().simplified ();
  for (auto request : Request)
    if (request->url == url)
    {
      request->error = error;
      request->finished = status;
      return;
    }

  for (auto request : Request)
    if (request->actualUrl == url)
    {
      request->error = error;
      request->finished = status;
      return;
    }
}

// set actual url
void
NetService::setActualUrl_core (QString url, QString aurl)
{
  url = url.trimmed ().simplified ();
  aurl = aurl.trimmed ().simplified ();
  for (auto request : Request)
    if (request->url == url)
      request->actualUrl = aurl;
}

// remove request from vector
void
NetService::delRequest_core (QString url)
{
  int reqid = -1;

  if (Request.size () < 1)
    return;

  url = url.trimmed ().simplified ();
  for (int counter = 0; counter < Request.size (); counter ++)
    if (Request[counter]->url == url)
      reqid = counter;

  if (reqid != -1)
  {
#ifdef DEBUG
    qDebug () << objectName () << "Deleted:" << Request.size () << url;
#endif

    delete Request[reqid];
    Request.removeAt (reqid);
  }
}

// downloads the url and returns a string with a temporary filename
CG_ERR_RESULT
NetService::httpGET (const QString url, QFile & tempFile, Cookies *cookies)
{
#ifdef DEBUG
  qDebug () << Q_FUNC_INFO << url;
#endif
  
  CG_ERR_RESULT result = CG_ERR_OK;

  unique_ptr<QByteArray> replyBytes (new (std::nothrow) QByteArray);
  if (replyBytes.get () == nullptr)
    return CG_ERR_NOMEM;

  const QUrl qurl = QUrl::fromEncoded(url.toLatin1 ());
  if (!addRequest (qurl.toString (), static_cast <void *> (replyBytes.get ())))
    return CG_ERR_REQUEST_PENDING;

  tempFile.resize (0);
  unique_ptr <QNetworkRequest> request (new (std::nothrow) QNetworkRequest);
  if (request.get () == nullptr)
    return CG_ERR_NOMEM;

  request.get ()->setSslConfiguration(QSslConfiguration::defaultConfiguration());
  request.get ()->setUrl(qurl);
  request.get ()->setRawHeader("User-Agent", header.toLatin1 ());
  if (cookie.size () > 0)
    request.get ()->setRawHeader("cookie: ", cookie.toLatin1 ());
  
  QNetworkAccessManager 
    *httpAccessManager = new (std::nothrow) QNetworkAccessManager;
  
  if (httpAccessManager == nullptr)
    return CG_ERR_NOMEM;

  registerNetworkAccessManager (httpAccessManager);

  connect (httpAccessManager, SIGNAL (finished (QNetworkReply *)), this,
           SLOT (httpFinished (QNetworkReply *)));
  
  QNetworkReply *reply = httpAccessManager->get (*request.get ());
  connect (reply, SIGNAL (error (QNetworkReply::NetworkError)), this,
           SLOT (netError (QNetworkReply::NetworkError)));
  
  Q_DECL_CONSTEXPR qint32 invl = 20;
  qint32 tcounter = 0;
  while (tcounter < (timeout * 1000))
  {
    Sleeper::msleep(static_cast <unsigned long> (invl));
    qApp->sendPostedEvents (/*reply, 0*/);
    if (checkRequestStatus (qurl.toString (), result))
    {
      delRequest (qurl.toString ());
      tcounter = timeout * 1000;
    }
    tcounter += invl;
  }

  if (result == CG_ERR_OK)
  {
    // get the cookies
    if (cookies != nullptr)
    {
      const QVariant cookieVar = reply->header(QNetworkRequest::SetCookieHeader);
      if (cookieVar.isValid())
        *cookies = cookieVar.value < QList<QNetworkCookie> > ();
    }

    if (tempFile.write (*replyBytes.get ()) == -1)
      result = CG_ERR_WRITE_FILE;
  }
  
  delRequest (qurl.toString ());

  return result;
}

// callback of access manager
void
NetService::httpFinished(QNetworkReply *reply)
{
  
  const QString url = reply->url ().toString ();
  
  setRequestStatus (url, 1, CG_ERR_OK);
  	
  const QVariant replyStatus =
    reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);

  if (replyStatus.isNull() == true)
  {
    setRequestStatus (url, 1, CG_ERR_NO_DATA);
  }
  else
  if (replyStatus.isValid () == false)
  {
    setRequestStatus (url, 1, CG_ERR_INVALID_DATA);
  }
  else
  if (replyStatus == 200) // Ok
  {
    QByteArray *replyBytes = static_cast <QByteArray *> (getReplyBuffer (url));

    if (replyBytes != nullptr)
      *replyBytes = reply->readAll ();
    else
    {
      setRequestStatus (url, 1, CG_ERR_BUFFER_NOTFOUND);
    }
  }
  else 
  if (replyStatus == 301) // redirect
  {
    const QString redirectUrl =
      reply->attribute (QNetworkRequest::RedirectionTargetAttribute).toString();
    setActualUrl (url, redirectUrl);

    unique_ptr <QNetworkRequest> request (new QNetworkRequest);
    request.get ()->setSslConfiguration(QSslConfiguration::defaultConfiguration());
    request.get ()->setUrl (QUrl(redirectUrl));
    request.get ()->setRawHeader("User-Agent", header.toLatin1 ());
    reply->manager ()->get (*request.get ());
  }
  
  reply->deleteLater ();
}

// receive network error code
void
NetService::netError (QNetworkReply::NetworkError code)
{
  QNetworkReply *reply = qobject_cast <QNetworkReply *> (QObject::sender());

  if (reply == nullptr)
    return;

  if (code == QNetworkReply::TimeoutError)
    setRequestStatus (reply->url ().toString (), 1, CG_ERR_NETWORK_TIMEOUT);
  else
    setRequestStatus (reply->url ().toString (), 1, CG_ERR_NETWORK);
}

// register QNetworkAccessManager
void 
NetService::registerNetworkAccessManager (QNetworkAccessManager *manager)
{
  Mutex->lock ();
  
  QDateTime ts;
  
  if (NetGC.size () > 0)
  {
	if (ts.currentMSecsSinceEpoch() - NetGC[0].timestamp> 120000)
	{
	  if (NetGC[0].manager != nullptr)
	    delete  NetGC[0].manager;
	  NetGC.remove (0);     
	}  
  }	  
  
  if (NetGC.size () > 0)
  {
	if (ts.currentMSecsSinceEpoch() - NetGC[0].timestamp > 120000)
	{
	  if (NetGC[0].manager != nullptr)
	    delete  NetGC[0].manager;
	  NetGC.remove (0);     
	}  
  }
  
  NetGCRec mrec;
  mrec.manager = manager;
  mrec.timestamp = ts.currentMSecsSinceEpoch();
  NetGC.append (mrec); 
  
  Mutex->unlock ();
}	
