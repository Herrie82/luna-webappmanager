/*
 * Copyright (C) 2013 Simon Busch <morphis@gravedo.de>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>
 */

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QQuickView>
#include <QFile>
#include <QFileInfo>
#include <QUrl>
#include <QtWebEngineVersion>
#include <QMap>

#include <luna-service2/lunaservice.h>
#include <luna-service2++/message.hpp>
#include <luna-service2++/call.hpp>
#include <luna-service2++/handle.hpp>

#include <LocalePreferences.h>

#include "../webapplication.h"
#include "../webapplicationwindow.h"
#include "../webappmanager.h"
#include "../webappmanagerservice.h"
#include "../systemtime.h"
#include "palmsystemextension.h"
#include "deviceinfo.h"

namespace luna
{

// little utility to return one application service per iAppId
class LunaAppServicesManager {
public:
    LS::Handle &getAppService(const QString &iAppServiceName, const QString &iAppId) {
        if (!mapAppServiceName.contains(iAppId)) {
            try {
                mapAppServiceName[iAppId] = new LS::Handle(iAppServiceName.toUtf8().constData(), iAppId.toUtf8().constData());
            }  catch (LS::Error &error) {
                mapAppServiceName[iAppId] = new LS::Handle();
            }
        }

        return *(mapAppServiceName[iAppId]);
    }

    ~LunaAppServicesManager() {
        for(auto h: mapAppServiceName.values()) {
            if(h) delete h;
        }
    }
private:
    QMap<QString, LS::Handle*> mapAppServiceName;
} _lunaAppServicesManager;

PalmSystemExtension::PalmSystemExtension(WebApplicationWindow *applicationWindow, QObject *parent) :
    BaseExtension("PalmSystem", applicationWindow, parent),
    mApplicationWindow(applicationWindow),
    mLunaAppHandle(_lunaAppServicesManager.getAppService(applicationWindow->application()->identifier(), applicationWindow->application()->id()))
{
    applicationWindow->registerUserScript(QString("://extensions/PalmSystem.js"), false);
    applicationWindow->registerUserScript(QString("://extensions/PalmSystemBridge.js"), true);
    if( applicationWindow->isMainWindow() )
    {
        connect(applicationWindow->application(), SIGNAL(parametersChanged(bool)), this, SIGNAL(launchParamsChanged(bool)));
    }

    connect(applicationWindow, SIGNAL(activeChanged()), this, SIGNAL(isActivatedChanged()));
    
    try {
        mLunaAppHandle.attachToLoop(g_main_context_default());
    }  catch (LS::Error &error) {}
}

LS::Handle &PalmSystemExtension::getLunaHandle()
{
    WebAppManager *pWebAppManager = (WebAppManager*)qGuiApp;
    if(pWebAppManager && pWebAppManager->getService()) return pWebAppManager->getService()->getServiceHandle();
    
    // fallback on application service
    return mLunaAppHandle;
}

void PalmSystemExtension::stageReady()
{
    qDebug() << __PRETTY_FUNCTION__;
    mApplicationWindow->stageReady();
}

void PalmSystemExtension::activate()
{
    qDebug() << __PRETTY_FUNCTION__;
    mApplicationWindow->focus();
}

void PalmSystemExtension::deactivate()
{
    qDebug() << __PRETTY_FUNCTION__;
    mApplicationWindow->unfocus();
}

void PalmSystemExtension::stagePreparing()
{
    qDebug() << __PRETTY_FUNCTION__;
    mApplicationWindow->stagePreparing();
}

void PalmSystemExtension::show()
{
    qDebug() << __PRETTY_FUNCTION__;
    mApplicationWindow->show();
}

void PalmSystemExtension::hide()
{
    qDebug() << __PRETTY_FUNCTION__;
    mApplicationWindow->hide();
}

void PalmSystemExtension::setWindowProperties(const QString &properties)
{
    qDebug() << __PRETTY_FUNCTION__ << properties;
}

void PalmSystemExtension::enableFullScreenMode(bool enable)
{
    qDebug() << __PRETTY_FUNCTION__ << enable;

    QString appId = mApplicationWindow->application()->identifier();
    QString enableStr = enable ? "true" : "false";

    try {
        LS::Call call = getLunaHandle().callOneReply("luna://org.webosports.luna/enableFullScreenMode",
                                                    QString("{\"enable\": %1}").arg(enableStr).toUtf8().constData(),
                                                    appId.toUtf8().constData());
    }  catch (LS::Error &error) {}
}

void PalmSystemExtension::removeBannerMessage(int id)
{
    qDebug() << __PRETTY_FUNCTION__;

    QString appId = mApplicationWindow->application()->identifier();

    try {
        LS::Call call = getLunaHandle().callOneReply("luna://org.webosports.notifications/close",
                                                    QString("{\"id\":\"%1\"}").arg(appId).toUtf8().constData(),
                                                    appId.toUtf8().constData());
    }  catch (LS::Error &error) {}
}

void PalmSystemExtension::clearBannerMessages()
{
    qDebug() << __PRETTY_FUNCTION__;

    QString appId = mApplicationWindow->application()->identifier();

    try {
        LS::Call call = getLunaHandle().callOneReply("luna://org.webosports.notifications/closeAll",
                                                    "{}", appId.toUtf8().constData());
    }  catch (LS::Error &error) {}
}

void PalmSystemExtension::keepAlive(bool keep)
{
    qDebug() << __PRETTY_FUNCTION__ << keep;
    mApplicationWindow->setKeepAlive(keep);
}

void PalmSystemExtension::markFirstUseDone()
{
    QFile firstUseMarker("/var/luna/preferences/ran-first-use");
    firstUseMarker.open(QIODevice::ReadWrite);
    firstUseMarker.close();
}


QString PalmSystemExtension::launchParams()
{
    return mApplicationWindow->application()->parameters();
}

bool    PalmSystemExtension::hasAlphaHole()
{
    return false;
}
void    PalmSystemExtension::setHasAlphaHole(bool iVal)
{
}

QString PalmSystemExtension::locale()
{
    return LocalePreferences::instance()->locale();
}

QString PalmSystemExtension::localeRegion()
{
    return LocalePreferences::instance()->localeRegion();
}

QString PalmSystemExtension::timeFormat()
{
    return LocalePreferences::instance()->timeFormat();
}

QString PalmSystemExtension::timeZone()
{
    return SystemTime::instance()->timezone();
}

bool    PalmSystemExtension::isMinimal()
{
    return false;
}

QString PalmSystemExtension::identifier()
{
    return mApplicationWindow->application()->identifier();
}

QString PalmSystemExtension::screenOrientation()
{
    return "";
}

QString PalmSystemExtension::windowOrientation()
{
    return "";
}

void    PalmSystemExtension::setWindowOrientation(QString iVal)
{
}

QString PalmSystemExtension::specifiedWindowOrientation()
{
    return "";
}

QString PalmSystemExtension::videoOrientation()
{
    return "";
}

QString PalmSystemExtension::deviceInfo()
{
    return DeviceInfo::instance()->jsonString();
}

bool    PalmSystemExtension::isActivated()
{
    return mApplicationWindow->active();
}

int     PalmSystemExtension::activityId()
{
    return mApplicationWindow->application()->activityId();
}

QString PalmSystemExtension::phoneRegion()
{
    return LocalePreferences::instance()->phoneRegion();
}

QString PalmSystemExtension::version()
{
    return QString(QTWEBENGINE_VERSION_STR);
}

QString PalmSystemExtension::getResource(const QString&resPath, const QString &)
{
    qDebug() << __PRETTY_FUNCTION__ << resPath;

    QString path = resPath;
    if (path.startsWith("file://"))
        path = path.right(path.size() - 7);

    if (!mApplicationWindow->application()->validateResourcePath(path)) {
        qDebug() << "WARNING: Access to path" << path << "is not allowed";
        return QString("");
    }

    QFile file(path);
    if (!file.open(QIODevice::ReadOnly))
        return QString("");

    QByteArray data = file.readAll();

    return data;
}

QString PalmSystemExtension::getIdentifierForFrame(const QString&id, const QString &url)
{
    qDebug() << __PRETTY_FUNCTION__ << id << ", " << url;

    return mApplicationWindow->getIdentifierForFrame(id, url);
}

QString PalmSystemExtension::addBannerMessage(const QString &msgTitle, const QString &launchParams,
                                              const QString &msgIconUrl, const QString &soundClass,
                                              const QString &msgSoundFile, int duration,
                                              bool doNotSuppress)
{
    qDebug() << __PRETTY_FUNCTION__ << msgTitle << ":" << launchParams;

    QString appId = mApplicationWindow->application()->identifier();

    QString iconUrl = msgIconUrl;
    QString soundFile = msgSoundFile;
    QJsonObject notificationParams;
    
    notificationParams.insert("title", msgTitle);
    notificationParams.insert("launchParams", launchParams);
    notificationParams.insert("iconUrl", iconUrl);
    notificationParams.insert("soundClass", soundClass);
    notificationParams.insert("soundFile", soundFile);
    notificationParams.insert("duration", duration);
    notificationParams.insert("doNotSuppress", doNotSuppress);
    notificationParams.insert("expireTimeout", "0");

    QJsonDocument document(notificationParams);
    QJsonObject response;

    try {
        LS::Call call = getLunaHandle().callOneReply("luna://org.webosports.notifications/create",
                                                    document.toJson().constData(),
                                                    appId.toUtf8().constData());
        LS::Message message(call.get(1000));

        response = QJsonDocument::fromJson(message.getPayload()).object();
    }  catch (LS::Error &error) {}

    if (!response.contains("id"))
        return QString("");

    return QString("%1").arg(response.value("id").toInt());
}

void PalmSystemExtension::LS2Call(int callId, int bridgeId, const QString &uri, const QString &payload)
{
    PalmServiceBridgeObject &lBridgeObject = mListBridges[bridgeId]; // this will create a new PalmServiceBridgeObject if needed
    lBridgeObject.bridgeId = bridgeId;
    lBridgeObject.callId = callId;
    lBridgeObject.palmExt = this;

    try {
        lBridgeObject.currentBridgeCall.reset(new LS::Call(mLunaAppHandle.callMultiReply(uri.toLatin1().data(),
                                                                                         payload.toLatin1().data(),
                                                                                         &replyCallback, &lBridgeObject)));
    }  catch (LS::Error &error) {}
}

void PalmSystemExtension::LS2Cancel(int bridgeId)
{
    PalmServiceBridgeObject &lBridgeObject = mListBridges[bridgeId]; // this will create a new PalmServiceBridgeObject if needed
    lBridgeObject.currentBridgeCall->cancel();
}

bool PalmSystemExtension::PalmServiceBridgeObject::handleReply(LSHandle *sh, LSMessage *reply)
{
    if(reply && palmExt)
    {
        LS::Message _reply(reply);
        palmExt->callback(callId, true, true, _reply.getPayload());
    }

    return true;
}

bool PalmSystemExtension::replyCallback(LSHandle* sh, LSMessage* reply, void* context)
{
    if (context)
    {
        PalmServiceBridgeObject *pBridgeObject = static_cast<PalmServiceBridgeObject *>(context);
        pBridgeObject->handleReply(sh, reply);
    }
    return true;
}

} // namespace luna
