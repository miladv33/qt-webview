/****************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** BSD License Usage
** Alternatively, you may use this file under the terms of the BSD license
** as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include <QtCore/QUrl>
#include <QtCore/QCommandLineOption>
#include <QtCore/QCommandLineParser>
#include <QGuiApplication>
#include <QStyleHints>
#include <QScreen>
#include <QQmlApplicationEngine>
#include <QtQml/QQmlContext>
#include <QtWebView/QtWebView>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QNetworkDiskCache>
#include <QNetworkDiskCache>
#include <QDesktopServices>
#include <QQmlNetworkAccessManagerFactory>


class MyNetworkAccessManager : public QNetworkAccessManager
{
public:
    MyNetworkAccessManager(QObject *parent) : QNetworkAccessManager(parent) { }

protected:
    /**
     * This function is called whenever a request is made. We override it to set the cache to always
     * cache.
     *
     * @param operation The type of operation to be performed.
     * @param request The request that is being sent.
     * @param outgoingData This is the data that is sent to the server.
     *
     * @return A QNetworkReply object.
     */
    /**
     * This function is called whenever a request is made. It is called by the QNetworkAccessManager.
     * It is called with the operation, the request, and the outgoing data. It returns a QNetworkReply.
     * The function is overridden to set the cache to always cache.
     *
     * @param operation The type of operation to be performed.
     * @param request The request that is being sent.
     * @param outgoingData This is the data that is sent to the server.
     *
     * @return A QNetworkReply object.
     */
    QNetworkReply *createRequest(Operation operation, const QNetworkRequest &request, QIODevice *outgoingData = nullptr) override
    {
        /* Creating a new QNetworkRequest object and setting it to the request that is passed in. */
        QNetworkRequest cacheRequest(request);
        cacheRequest.setAttribute(QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::AlwaysCache);
        return QNetworkAccessManager::createRequest(operation, cacheRequest, outgoingData);
    }
};

class MyNetworkAccessManagerFactory : public QQmlNetworkAccessManagerFactory
{
public:
    /**
     * This function creates a new QNetworkAccessManager object, sets the cache directory, and returns
     * the object.
     *
     * @param parent The parent QObject of the QNetworkAccessManager.
     *
     * @return A pointer to a QNetworkAccessManager object.
     */
    QNetworkAccessManager *create(QObject *parent) override
    {
        QNetworkAccessManager *nam = new MyNetworkAccessManager(parent);
        QNetworkDiskCache *cache = new QNetworkDiskCache(nam);
//        cache->setCacheDirectory(QDesktopServices::storageLocation(QDesktopServices::CacheLocation));
        qDebug()<<cache->cacheDirectory();
        nam->setCache(cache);
        return nam;
    }
};


// Workaround: As of Qt 5.4 QtQuick does not expose QUrl::fromUserInput.
class Utils : public QObject {
    Q_OBJECT
public:
    Utils(QObject *parent = nullptr) : QObject(parent) { }
    Q_INVOKABLE static QUrl fromUserInput(const QString& userInput);
};

QUrl Utils::fromUserInput(const QString& userInput)
{
    if (userInput.isEmpty())
        return QUrl::fromUserInput("about:blank");
    const QUrl result = QUrl::fromUserInput(userInput);
    return result.isValid() ? result : QUrl::fromUserInput("about:blank");
}

#include "main.moc"

int main(int argc, char *argv[])
{
//! [0]
    QtWebView::initialize();
    QGuiApplication app(argc, argv);
//! [0]
    QGuiApplication::setApplicationDisplayName(QCoreApplication::translate("main",
                                                                           "QtWebView Example"));
    QCommandLineParser parser;
    QCoreApplication::setApplicationVersion(QT_VERSION_STR);
    parser.setApplicationDescription(QGuiApplication::applicationDisplayName());
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addPositionalArgument("url", "The initial URL to open.");
    QStringList arguments = app.arguments();
    parser.process(arguments);
    const QString initialUrl = parser.positionalArguments().isEmpty() ?
        QStringLiteral("https://github.com") : parser.positionalArguments().first();

    QQmlApplicationEngine engine;
    QQmlContext *context = engine.rootContext();
    context->setContextProperty(QStringLiteral("utils"), new Utils(&engine));
    context->setContextProperty(QStringLiteral("initialUrl"),
                                Utils::fromUserInput(initialUrl));
    QRect geometry = QGuiApplication::primaryScreen()->availableGeometry();
    if (!QGuiApplication::styleHints()->showIsFullScreen()) {
        const QSize size = geometry.size() * 4 / 5;
        const QSize offset = (geometry.size() - size) / 2;
        const QPoint pos = geometry.topLeft() + QPoint(offset.width(), offset.height());
        geometry = QRect(pos, size);
    }
    context->setContextProperty(QStringLiteral("initialX"), geometry.x());
    context->setContextProperty(QStringLiteral("initialY"), geometry.y());
    context->setContextProperty(QStringLiteral("initialWidth"), geometry.width());
    context->setContextProperty(QStringLiteral("initialHeight"), geometry.height());
     engine.setNetworkAccessManagerFactory(new MyNetworkAccessManagerFactory);

    engine.load(QUrl(QStringLiteral("qrc:/main.qml")));
    if (engine.rootObjects().isEmpty())
        return -1;

    return app.exec();
}
