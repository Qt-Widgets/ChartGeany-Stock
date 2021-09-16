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

#pragma once

#include <QObject>
#include <QFile>
#include <QAtomicInt>
#include <QNetworkReply>
#include <QNetworkCookie>
#include <QMutex>

#include "defs.h"

// network request structure
struct NetRequestClass
{
  QString 		url;			// the url
  QString 		actualUrl;	    // the actual url (eg after redirect)
  void    		*replyBytes;    // network reply buffer
  qint16	 	finished;		// operation status: 0 false/finished, 1 true/running
  qint16	    error;			// error
};
Q_DECLARE_TYPEINFO (NetRequestClass, Q_MOVABLE_TYPE);
typedef QList <NetRequestClass *> NetRequestVector;
typedef QList <QNetworkCookie> Cookies;

class NetService : public QObject
{
  Q_OBJECT
  
public:
  explicit NetService (qint16  nettimeout, QString httpHeader, QObject *parent = 0); // constructor
  ~NetService (void);	     				 // destructor
  
  // downloads the url content in tempFile
  CG_ERR_RESULT httpGET (const QString url, QFile & tempFile, Cookies *cookies = 0);
  void setCookie (QString s) { cookie = s; }; // set's the cookie for http header
  
  // sets acceptRequests status
  void setAcceptRequestStatus (bool status)
  {
	Mutex->lock ();
	acceptRequests = status;
	Mutex->unlock ();
  }
  
private:
  struct NetGCRec
  {
	QNetworkAccessManager *manager;
	qint64 timestamp;   
  };
  	 
  QVector<NetGCRec> NetGC;			// QNetworkAccessManager garbage collector 	  
  QMutex *Mutex;					// protects Request from concurrent access
  NetRequestVector Request;			// list of all requests
  QString header;					// http header
  QString cookie;					// cookie for http header
  qint16  timeout;					// network timeout in seconds
  bool	  acceptRequests;			// accept new requests
  
  // register a new QNetworkAccessManager with the garbage collector
  void registerNetworkAccessManager (QNetworkAccessManager *manager);
  
  // returns true on success, false on failure
  bool addRequest_core (QString url, void *replyBuffer); // add request in vector
  bool addRequest (QString url, void *replyBuffer)
  {
	if (!Mutex->tryLock (200)) return false;
	bool result = addRequest_core (url, replyBuffer);
	Mutex->unlock ();
	return result;    
  }
  
  // returns the error code
  int checkRequestStatus_core (QString url, int & error);  // check request's status
  int checkRequestStatus (QString url, int & error)
  {
	Mutex->lock ();
	int result = checkRequestStatus_core (url, error);
	Mutex->unlock ();
	return result;  
  }	  
  
  void *getReplyBuffer_core (QString url); // get the reply buffer;
  void *getReplyBuffer (QString url)
  {
	Mutex->lock ();
	void *result = getReplyBuffer_core (url);
	Mutex->unlock ();
	return result;    
  }	  
  
  void setRequestStatus_core (QString url, const int status, const int error); // set the status of a request
  void setRequestStatus (QString url, int status, int error)
  {
	Mutex->lock ();
	setRequestStatus_core (url, status, error);
	Mutex->unlock ();
  }	  
  
  void setActualUrl_core (QString url, QString aurl); // set the actual url
  void setActualUrl (QString url, QString aurl)
  {
	Mutex->lock ();
	setActualUrl_core (url, aurl);
	Mutex->unlock ();
  }	  
  
  void delRequest_core (QString url); // remove request from vector
  void delRequest (QString url)
  {
	Mutex->lock ();
	delRequest_core (url);
	Mutex->unlock ();
  }
  
private slots:
  void httpFinished (QNetworkReply *checkreply); // callback of access manager
  void netError (QNetworkReply::NetworkError code); // receive network error code
};
