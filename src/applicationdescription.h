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

#ifndef APPLICATIONDESCRIPTION_H
#define APPLICATIONDESCRIPTION_H

#include <QObject>
#include <QString>
#include <QUrl>

namespace luna
{

class ApplicationDescription : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString id READ id CONSTANT)
    Q_PROPERTY(QString title READ title CONSTANT)
    Q_PROPERTY(QString icon READ icon CONSTANT)
    Q_PROPERTY(QUrl entryPoint READ entryPoint CONSTANT)
    Q_PROPERTY(bool isHeadLess READ isHeadLess CONSTANT)

public:
    ApplicationDescription();
    ApplicationDescription(const QString &data);
    virtual ~ApplicationDescription();

    QString id() const;
    QString title() const;
    QString icon() const;
    QUrl entryPoint() const;
    bool isHeadLess() const;

private:
    QString mId;
    QString mTitle;
    QString mIcon;
    QUrl mEntryPoint;
    bool mIsHeadLess;
};

} // namespace luna

#endif // APPLICATIONDESCRIPTION_H
