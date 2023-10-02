/**
 *  OSM
 *  Copyright (C) 2019  Pavel Smokotnin

 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.

 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.

 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef SETTINGS_H
#define SETTINGS_H

#include <QSettings>
#include <QCoreApplication>

class Settings : public QObject
{
    Q_OBJECT

public:
    explicit Settings(const QString &group = "", QObject *parent = nullptr);
    QSettings *parent()
    {
        return dynamic_cast<QSettings *>(QObject::parent());
    }
    Q_INVOKABLE void setValue(const QString &key, const QVariant &value);
    Q_INVOKABLE QVariant value(const QString &key, const QVariant &defaultValue = QVariant());
    Q_INVOKABLE Settings *getGroup(const QString &groupName);
    Q_INVOKABLE Settings *getSubGroup(const QString &groupName);

    template <typename C, typename T>
    QVariant reactValue(
        const QString &key,
        const C *sender,
        void (C::* signal)(T),
        const QVariant &defaultValue = QVariant())
    {
        connect(sender, signal, [t = this, key = key](const T & newvalue) {
            t->setValue(key, newvalue);
        });
        return value(key, defaultValue);
    }

    void flush();

protected:
    class group_guard
    {
        QString m_guarded;

    public:
        explicit group_guard(const QString &name): m_guarded(name)
        {
            if (!m_guarded.isEmpty())
                m_settings->beginGroup(m_guarded);
        }
        ~group_guard()
        {
            if (!m_guarded.isEmpty())
                m_settings->endGroup();
        }
    };

private:
    static QSettings *m_settings;
    QString m_group;
};

#endif // SETTINGS_H
