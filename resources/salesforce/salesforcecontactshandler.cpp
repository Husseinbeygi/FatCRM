/*
  This file is part of FatCRM, a desktop application for SugarCRM written by KDAB.

  Copyright (C) 2015-2017 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com
  Authors: David Faure <david.faure@kdab.com>
           Michel Boyer de la Giroday <michel.giroday@kdab.com>
           Kevin Krammer <kevin.krammer@kdab.com>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "salesforcecontactshandler.h"
#include "salesforcesoap.h"
using namespace KDSoapGenerated;

#include <Akonadi/Collection>

#include <KABC/Addressee>

#include <KLocale>

#include <QHash>

typedef QString(*valueGetter)(const KABC::Addressee &);
typedef void (*valueSetter)(const QString &, KABC::Addressee &);

static QString getId(const KABC::Addressee &addressee)
{
    return addressee.uid();
}

static void setId(const QString &value, KABC::Addressee &addressee)
{
    addressee.setUid(value);
}

static QString getFirstName(const KABC::Addressee &addressee)
{
    return addressee.givenName();
}

static void setFirstName(const QString &value, KABC::Addressee &addressee)
{
    addressee.setGivenName(value);
}

static QString getLastName(const KABC::Addressee &addressee)
{
    return addressee.familyName();
}

static void setLastName(const QString &value, KABC::Addressee &addressee)
{
    addressee.setFamilyName(value);
}

static QString getTitle(const KABC::Addressee &addressee)
{
    return addressee.title();
}

static void setTitle(const QString &value, KABC::Addressee &addressee)
{
    addressee.setTitle(value);
}

static QString getDepartment(const KABC::Addressee &addressee)
{
    return addressee.department();
}

static void setDepartment(const QString &value, KABC::Addressee &addressee)
{
    addressee.setDepartment(value);
}

static QString getEmail1(const KABC::Addressee &addressee)
{
    return addressee.preferredEmail();
}

static void setEmail1(const QString &value, KABC::Addressee &addressee)
{
    addressee.insertEmail(value, true);
}

static QString getHomePhone(const KABC::Addressee &addressee)
{
    return addressee.phoneNumber(KABC::PhoneNumber::Home).number();
}

static void setHomePhone(const QString &value, KABC::Addressee &addressee)
{
    addressee.insertPhoneNumber(KABC::PhoneNumber(value, KABC::PhoneNumber::Home));
}

static QString getWorkPhone(const KABC::Addressee &addressee)
{
    return addressee.phoneNumber(KABC::PhoneNumber::Work).number();
}

static void setWorkPhone(const QString &value, KABC::Addressee &addressee)
{
    addressee.insertPhoneNumber(KABC::PhoneNumber(value, KABC::PhoneNumber::Work));
}

static QString getMobilePhone(const KABC::Addressee &addressee)
{
    return addressee.phoneNumber(KABC::PhoneNumber::Cell).number();
}

static void setMobilePhone(const QString &value, KABC::Addressee &addressee)
{
    addressee.insertPhoneNumber(KABC::PhoneNumber(value, KABC::PhoneNumber::Cell));
}

class ContactAccessorPair
{
public:
    ContactAccessorPair(valueGetter get, valueSetter set)
        : isAvailable(false), getter(get), setter(set) {}

public:
    bool isAvailable;
    valueGetter getter;
    valueSetter setter;
};

SalesforceContactsHandler::SalesforceContactsHandler()
    : SalesforceModuleHandler(QLatin1String("Contact")),
      mAccessors(new ContactAccessorHash)
{
    // interestingly, if we don't specifically request Id in queries, the resulting
    // XML does contain empty Id elements. If we request Id, we get it twice, but that's better
    // than never
    mAccessors->insert(QLatin1String("Id"), ContactAccessorPair(getId, setId));

    mAccessors->insert(QLatin1String("FirstName"), ContactAccessorPair(getFirstName, setFirstName));
    mAccessors->insert(QLatin1String("LastName"), ContactAccessorPair(getLastName, setLastName));

    mAccessors->insert(QLatin1String("Email"), ContactAccessorPair(getEmail1, setEmail1));
    mAccessors->insert(QLatin1String("Title"), ContactAccessorPair(getTitle, setTitle));
    mAccessors->insert(QLatin1String("Department"), ContactAccessorPair(getDepartment, setDepartment));
    mAccessors->insert(QLatin1String("HomePhone"), ContactAccessorPair(getHomePhone, setHomePhone));
    mAccessors->insert(QLatin1String("Phone"), ContactAccessorPair(getWorkPhone, setWorkPhone));
    mAccessors->insert(QLatin1String("MobilePhone"), ContactAccessorPair(getMobilePhone, setMobilePhone));
}

SalesforceContactsHandler::~SalesforceContactsHandler()
{
    delete mAccessors;
}

QStringList SalesforceContactsHandler::supportedFields() const
{
    return mAccessors->keys();
}

void SalesforceContactsHandler::setDescriptionResult(const TNS__DescribeSObjectResult &description)
{
    SalesforceModuleHandler::setDescriptionResult(description);

    const QSet<QString> fields = availableFields().toSet();

    ContactAccessorHash::iterator it    = mAccessors->begin();
    ContactAccessorHash::iterator endIt = mAccessors->end();
    for (; it != endIt; ++it) {
        it->isAvailable = fields.contains(it.key());

        if (!it->isAvailable) {
            kDebug() << "Disabling accessor pair for" << it.key()
                     << "because it is not part of the server's available fields";
        }
    }
}

Akonadi::Collection SalesforceContactsHandler::collection() const
{
    Akonadi::Collection contactCollection;
    contactCollection.setRemoteId(moduleName());
    contactCollection.setContentMimeTypes(QStringList() << KABC::Addressee::mimeType());
    contactCollection.setName(i18nc("@item folder name", "Contacts"));
    contactCollection.setRights(Akonadi::Collection::CanChangeItem |
                                Akonadi::Collection::CanCreateItem |
                                Akonadi::Collection::CanDeleteItem);

    return contactCollection;
}

void SalesforceContactsHandler::listEntries(const TNS__QueryLocator &locator, SforceService *soap)
{
    static QString queryString = QLatin1String("Select ") +
                                 QStringList(mAccessors->keys()).join(QLatin1String(", ")) +
                                 QLatin1String(" from Contact");   // without trailing 's'

    if (locator.value().isEmpty()) {
        TNS__Query query;
        query.setQueryString(queryString);
        soap->asyncQuery(query);
    } else {
        TNS__QueryMore query;
        query.setQueryLocator(locator);
        soap->asyncQueryMore(query);
    }
}

bool SalesforceContactsHandler::setEntry(const Akonadi::Item &item, SforceService *soap)
{
    if (!item.hasPayload<KABC::Addressee>()) {
        kError() << "item (id=" << item.id() << ", remoteId=" << item.remoteId()
                 << ", mime=" << item.mimeType() << ") is missing Addressee payload";
        return false;
    }

    ENS__SObject object;
    object.setType(QLatin1String("Contact"));

    // if there is an id add it, otherwise skip this field
    // no id will result in the contact being added
    if (!item.remoteId().isEmpty()) {
        object.setId(item.remoteId());
    }

    Q_ASSERT(item.hasPayload<KABC::Addressee>());
    const KABC::Addressee addressee = item.payload<KABC::Addressee>();

    QList<KDSoapValue> valueList;
    ContactAccessorHash::const_iterator it    = mAccessors->constBegin();
    ContactAccessorHash::const_iterator endIt = mAccessors->constEnd();
    for (; it != endIt; ++it) {
        // Id is already part of the object, we have the accessor for the query
        if (it.key() == QLatin1String("Id")) {
            continue;
        }

        if (it->isAvailable) {
            const QString value = it->getter(addressee);
            valueList << KDSoapValue(it.key(), value);
            kDebug() << "Upsert: name=" << it.key() << "value=" << value;
        }
    }

    object.setAny(valueList);

    TNS__Upsert upsert;
    upsert.setExternalIDFieldName(QLatin1String("Id"));
    upsert.setSObjects(QList<ENS__SObject>() << object);
    soap->asyncUpsert(upsert);

    return true;
}

Akonadi::Item::List SalesforceContactsHandler::itemsFromListEntriesResponse(const TNS__QueryResult &queryResult,
        const Akonadi::Collection &parentCollection)
{
    Akonadi::Item::List items;

    Q_FOREACH (const ENS__SObject &entry, queryResult.records()) {
        const QList<KDSoapValue> valueList = entry.any();
        if (valueList.isEmpty()) {
            kWarning() << "Contacts entry for id=" << entry.id().value() << "has no values";
            kDebug() << "fieldsToNull:" << entry.fieldsToNull();
            continue;
        }

        Akonadi::Item item;
        item.setRemoteId(entry.id().value());
        item.setParentCollection(parentCollection);
        item.setMimeType(KABC::Addressee::mimeType());

        KABC::Addressee addressee;
        addressee.setUid(entry.id().value());

        QList<KDSoapValue>::const_iterator it    = valueList.constBegin();
        QList<KDSoapValue>::const_iterator endIt = valueList.constEnd();
        for (; it != endIt; ++it) {
            ContactAccessorHash::const_iterator accessorIt = mAccessors->constFind(it->name());
            if (accessorIt != mAccessors->constEnd()) {
                if (accessorIt->isAvailable) {
                    accessorIt->setter(it->value().value<QString>(), addressee);
                }
            } else {
                kWarning() << "Contacts entry for id=" << entry.id().value()
                           << "has unknown value named" << it->name();
            }
        }

        item.setPayload<KABC::Addressee>(addressee);
        items << item;
    }

    kDebug() << "Query result had" << items.count() << "valid contact items";
    return items;
}
