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

#include <QtWidgets>
#include <QtNetwork>

#include "client.h"


Client::Client(QWidget *parent)
    : QDialog(parent)
    , hostCombo(new QComboBox)
    , portLineEdit(new QLineEdit)
    , labelImage(new QLabel)
    , getServerButton(new QPushButton(tr("Get Server")))
    , selectImage(new QPushButton(tr("Select Image")))
    , sendImage(new QPushButton(tr("Send Image")))
    , tcpSocket(new QTcpSocket(this))
    , nextHeaderBytePacketSize(0)

{
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    hostCombo->setEditable(true);
    // Find out name of this machine
    QString name = QHostInfo::localHostName();
    if (!name.isEmpty()) {
        hostCombo->addItem(name);
        QString domain = QHostInfo::localDomainName();
        if (!domain.isEmpty())
            hostCombo->addItem(name + QChar('.') + domain);
    }
    if (name != QLatin1String("localhost"))
        hostCombo->addItem(QString("localhost"));
    // find out IP addresses of this machine
    QList<QHostAddress> ipAddressesList = QNetworkInterface::allAddresses();
    // add non-localhost addresses
    for (int i = 0; i < ipAddressesList.size(); ++i) {
        if (!ipAddressesList.at(i).isLoopback())
            hostCombo->addItem(ipAddressesList.at(i).toString());
    }
    // add localhost addresses
    for (int i = 0; i < ipAddressesList.size(); ++i) {
        if (ipAddressesList.at(i).isLoopback())
            hostCombo->addItem(ipAddressesList.at(i).toString());
    }

    portLineEdit->setValidator(new QIntValidator(1, 65535, this));

    auto hostLabel = new QLabel(tr("&Server name:"));
    hostLabel->setBuddy(hostCombo);
    auto portLabel = new QLabel(tr("&Server port:"));
    portLabel->setBuddy(portLineEdit);

    statusLabel = new QLabel(tr("This examples requires that you run the "
                                "Server example as well."));

    getServerButton->setDefault(true);
    getServerButton->setEnabled(false);

    auto quitButton = new QPushButton(tr("Quit"));

    auto buttonBox = new QDialogButtonBox;
    buttonBox->addButton(getServerButton, QDialogButtonBox::ActionRole);
    buttonBox->addButton(quitButton, QDialogButtonBox::RejectRole);
    buttonBox->addButton(selectImage, QDialogButtonBox::ActionRole);
    buttonBox->addButton(sendImage, QDialogButtonBox::RejectRole);

    labelImage->setMaximumWidth(500);
    labelImage->setMaximumHeight(400);
    labelImage->setScaledContents(true);
    labelImage->setFixedSize(500,400);
    sendImage->setEnabled(false);


    connect(hostCombo, &QComboBox::editTextChanged,
            this, &Client::enableGetServerButton);
    connect(portLineEdit, &QLineEdit::textChanged,
            this, &Client::enableGetServerButton);
    connect(getServerButton, &QAbstractButton::clicked,
            this, &Client::requestNewServer);
    connect(sendImage, &QAbstractButton::clicked,
            this, &Client::slotSendToServer);
    connect(selectImage, &QAbstractButton::clicked,
            this, &Client::selectImageOnButton);
    connect(quitButton, &QAbstractButton::clicked,
            this, &QWidget::close);

    connect(tcpSocket, SIGNAL(connected()), SLOT(slotConnected()));

    connect(tcpSocket, SIGNAL(readyRead()), SLOT(slotReadyRead()));

    connect(tcpSocket, QOverload<QAbstractSocket::SocketError>::of(&QAbstractSocket::error),
            this,      &Client::slotError);



    QGridLayout *mainLayout = nullptr;
    if (QGuiApplication::styleHints()->showIsFullScreen() || QGuiApplication::styleHints()->showIsMaximized()) {
        auto outerVerticalLayout = new QVBoxLayout(this);
        outerVerticalLayout->addItem(new QSpacerItem(0, 0, QSizePolicy::Ignored, QSizePolicy::MinimumExpanding));
        auto outerHorizontalLayout = new QHBoxLayout;
        outerHorizontalLayout->addItem(new QSpacerItem(0, 0, QSizePolicy::MinimumExpanding, QSizePolicy::Ignored));
        auto groupBox = new QGroupBox(QGuiApplication::applicationDisplayName());
        mainLayout = new QGridLayout(groupBox);
        outerHorizontalLayout->addWidget(groupBox);
        outerHorizontalLayout->addItem(new QSpacerItem(0, 0, QSizePolicy::MinimumExpanding, QSizePolicy::Ignored));
        outerVerticalLayout->addLayout(outerHorizontalLayout);
        outerVerticalLayout->addItem(new QSpacerItem(0, 0, QSizePolicy::Ignored, QSizePolicy::MinimumExpanding));
    } else {
        mainLayout = new QGridLayout(this);
    }
    mainLayout->addWidget(hostLabel, 0, 0);
    mainLayout->addWidget(hostCombo, 0, 1);
    mainLayout->addWidget(portLabel, 1, 0);
    mainLayout->addWidget(portLineEdit, 1, 1);
    mainLayout->addWidget(statusLabel, 2, 0, 1, 2);
    mainLayout->addWidget(buttonBox, 3, 0, 1, 2);
    mainLayout->addWidget(labelImage, 4,0,2,3);

    setWindowTitle(QGuiApplication::applicationDisplayName());
    portLineEdit->setFocus(); //Устанавливает фокус на окне ввода порта

    QNetworkConfigurationManager manager;
    if (manager.capabilities() & QNetworkConfigurationManager::NetworkSessionRequired) {
        // Get saved network configuration
        QSettings settings(QSettings::UserScope, QLatin1String("QtProject"));
        settings.beginGroup(QLatin1String("QtNetwork"));
        const QString id = settings.value(QLatin1String("DefaultNetworkConfiguration")).toString();
        settings.endGroup();

        // If the saved network configuration is not currently discovered use the system default
        QNetworkConfiguration config = manager.configurationFromIdentifier(id);
        if ((config.state() & QNetworkConfiguration::Discovered) !=
            QNetworkConfiguration::Discovered) {
            config = manager.defaultConfiguration();
        }


        connect(networkSession, &QNetworkSession::opened, this, &Client::sessionOpened);

        getServerButton->setEnabled(false);
        statusLabel->setText(tr("Opening network session."));
        networkSession->open();
    }

}




void Client::slotReadyRead()
{

    QDataStream in(tcpSocket);
    in.setVersion(QDataStream::Qt_5_10);

    for(;;){
        if(!nextHeaderBytePacketSize)
        {
            if(tcpSocket->bytesAvailable() < sizeof(quint64))
            {
                break;
            }

        in>>nextHeaderBytePacketSize;

        }
    if(tcpSocket->bytesAvailable()<nextHeaderBytePacketSize)
    {
        break;
    }

        QString nextString;
        QPixmap nextImage;
        in >> nextString >> nextImage;

        if(!nextString.isNull())
        {
        currentText = nextString;
        statusLabel->setText(currentText);
        }
        if(!nextImage.isNull())
        {
        currentImage = nextImage;
        labelImage->setPixmap(currentImage);
        sendImage->setEnabled(true);
        }

        nextHeaderBytePacketSize = 0;
        getServerButton->setEnabled(true);
    }
}

void Client::slotSendToServer()
{
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_5_10);
    out << quint64(0)<<QString("Hello, this pic with client") << currentImage;
    out.device()->seek(0);
    out<<quint64(block.size()- sizeof(quint64));

    tcpSocket->write(block);
    statusLabel->setText("Data send!");

}

void Client::slotError(QAbstractSocket::SocketError error)
{
    QString strError = "Error: " + (error==QAbstractSocket::HostNotFoundError ? "Host: not found " :
                                    error == QAbstractSocket::RemoteHostClosedError ? "Host closed connect ":
                                    error==QAbstractSocket::ConnectionRefusedError ? "connection was refused. ":
                                    QString(tcpSocket->errorString())
                                    );
    statusLabel->setText(strError);
    getServerButton->setEnabled(true);
}

void Client::requestNewServer()
{

    getServerButton->setEnabled(false);
    tcpSocket->abort();
    tcpSocket->connectToHost(hostCombo->currentText(), portLineEdit->text().toInt());
}



void Client::enableGetServerButton()
{
    getServerButton->setEnabled((!networkSession || networkSession->isOpen()) &&
                                 !hostCombo->currentText().isEmpty() &&
                                 !portLineEdit->text().isEmpty());

}

void Client::sessionOpened()
{
    // Save the used configuration
    QNetworkConfiguration config = networkSession->configuration();
    QString id;
    if (config.type() == QNetworkConfiguration::UserChoice)
        id = networkSession->sessionProperty(QLatin1String("UserChoiceConfiguration")).toString();
    else
        id = config.identifier();

    QSettings settings(QSettings::UserScope, QLatin1String("QtProject"));
    settings.beginGroup(QLatin1String("QtNetwork"));
    settings.setValue(QLatin1String("DefaultNetworkConfiguration"), id);
    settings.endGroup();

    statusLabel->setText(tr("This examples requires that you run the "
                            "Server example as well."));

    enableGetServerButton();
}

void Client::slotConnected()
{
    statusLabel->setText("Connect complete");
}

void Client::selectImageOnButton()
{
    QString filename = QFileDialog::getOpenFileName(this, "Select Image for send", QDir::currentPath());
    if(!filename.isEmpty())
    {
    labelImage->setPixmap(filename);
    currentImage=QPixmap(filename);
    statusLabel->setText("Picture select");
    sendImage->setEnabled(true);
    }
}





