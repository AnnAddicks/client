/*
 * Copyright (C) by Klaas Freitag <freitag@owncloud.com>
 * Copyright (C) by Daniel Molkentin <danimo@owncloud.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
 * for more details.
 */

#pragma once

#include "owncloudlib.h"
#include <QObject>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QPointer>
#include <QElapsedTimer>
#include <QDateTime>
#include <QTimer>
#include "accountfwd.h"

class QUrl;

namespace OCC {

class AbstractSslErrorHandler;

/**
 * @brief The AbstractNetworkJob class
 * @ingroup libsync
 */
class OWNCLOUDSYNC_EXPORT AbstractNetworkJob : public QObject {
    Q_OBJECT
public:
    explicit AbstractNetworkJob(AccountPtr account, const QString &path, QObject* parent = 0);
    virtual ~AbstractNetworkJob();

    virtual void start();

    AccountPtr account() const { return _account; }

    void setPath(const QString &path);
    QString path() const { return _path; }

    void setReply(QNetworkReply *reply);
    QNetworkReply* reply() const { return _reply; }

    void setIgnoreCredentialFailure(bool ignore);
    bool ignoreCredentialFailure() const { return _ignoreCredentialFailure; }

    /** Whether to handle redirects transparently.
     *
     * If true, a follow-up request is issued automatically when
     * a redirect is encountered. The finished() function is only
     * called if there are no more redirects (or there are problems
     * with the redirect).
     *
     * The transparent redirect following may be disabled for some
     * requests where custom handling is necessary.
     */
    void setFollowRedirects(bool follow);

    QByteArray responseTimestamp();

    qint64 timeoutMsec() { return _timer.interval(); }

public slots:
    void setTimeout(qint64 msec);
    void resetTimeout();
signals:
    void networkError(QNetworkReply *reply);
    void networkActivity();
protected:
    void setupConnections(QNetworkReply *reply);

    /** Initiate a network request, returning a QNetworkReply.
     *
     * Calls setReply() and setupConnections() on it.
     *
     * Takes ownership of the requestBody (to allow redirects).
     */
    QNetworkReply* sendRequest(const QByteArray& verb, const QUrl &url,
                               QNetworkRequest req = QNetworkRequest(),
                               QIODevice *requestBody = 0);

    // sendRequest does not take a relative path instead of an url,
    // but the old API allowed that. We have this undefined overload
    // to help catch usage errors
    QNetworkReply* sendRequest(const QByteArray& verb, const QString &relativePath,
                               QNetworkRequest req = QNetworkRequest(),
                               QIODevice *requestBody = 0);

    /// Creates a url for the account from a relative path
    QUrl makeAccountUrl(const QString& relativePath) const;

    /// Like makeAccountUrl() but uses the account's dav base path
    QUrl makeDavUrl(const QString& relativePath) const;

    int maxRedirects() const { return 10; }
    virtual bool finished() = 0;
    QByteArray    _responseTimestamp;
    bool          _timedout;  // set to true when the timeout slot is received

    // Automatically follows redirects. Note that this only works for
    // GET requests that don't set up any HTTP body or other flags.
    bool          _followRedirects;

    /** Helper to construct the HTTP verb used in the request
     *
     * Returns an empty QByteArray for UnknownOperation.
     */
    static QByteArray requestVerb(QNetworkReply* reply);

private slots:
    void slotFinished();
    virtual void slotTimeout();

protected:
    AccountPtr _account;

private:
    QNetworkReply* addTimer(QNetworkReply *reply);
    bool _ignoreCredentialFailure;
    QPointer<QNetworkReply> _reply; // (QPointer because the NetworkManager may be destroyed before the jobs at exit)
    QString _path;
    QTimer _timer;
    int _redirectCount;

    // Set by the xyzRequest() functions and needed to be able to redirect
    // requests, should it be required.
    //
    // Reparented to the currently running QNetworkReply.
    QPointer<QIODevice> _requestBody;
};

/**
 * @brief Internal Helper class
 */
class NetworkJobTimeoutPauser {
public:
    NetworkJobTimeoutPauser(QNetworkReply *reply);
    ~NetworkJobTimeoutPauser();
private:
    QPointer<QTimer> _timer;
};


/** Gets the SabreDAV-style error message from an error response.
 *
 * This assumes the response is XML with a 'error' tag that has a
 * 'message' tag that contains the data to extract.
 *
 * Returns a null string if no message was found.
 */
QString OWNCLOUDSYNC_EXPORT extractErrorMessage(const QByteArray& errorResponse);

/** Builds a error message based on the error and the reply body. */
QString OWNCLOUDSYNC_EXPORT errorMessage(const QString& baseError, const QByteArray& body);

} // namespace OCC


