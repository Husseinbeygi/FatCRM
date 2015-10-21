/*
  This file is part of FatCRM, a desktop application for SugarCRM written by KDAB.

  Copyright (C) 2015 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com
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

#include "notesrepository.h"

#include <Akonadi/Collection>
#include <Akonadi/CollectionStatistics>
#include <Akonadi/ItemFetchJob>
#include <Akonadi/ItemFetchScope>
#include <Akonadi/Monitor>

NotesRepository::NotesRepository(QObject *parent) :
    QObject(parent),
    mMonitor(0),
    mNotesLoaded(0),
    mEmailsLoaded(0)
{
}

void NotesRepository::clear()
{
    mNotesLoaded = 0;
    mEmailsLoaded = 0;
    mNotesHash.clear();
    mEmailsHash.clear();
    delete mMonitor;
    mMonitor = 0;
}

void NotesRepository::setNotesCollection(const Akonadi::Collection &collection)
{
    mNotesCollection = collection;
}

void NotesRepository::loadNotes()
{
    //kDebug() << "Loading" << mNotesCollection.statistics().count() << "notes";

    // load notes
    Akonadi::ItemFetchJob *job = new Akonadi::ItemFetchJob(mNotesCollection, this);
    configureItemFetchScope(job->fetchScope());
    connect(job, SIGNAL(itemsReceived(Akonadi::Item::List)),
            this, SLOT(slotNotesReceived(Akonadi::Item::List)));
}

QVector<SugarNote> NotesRepository::notesForOpportunity(const QString &id) const
{
    return mNotesHash.value(id);
}

void NotesRepository::slotNotesReceived(const Akonadi::Item::List &items)
{
    mNotesLoaded += items.count();
    foreach(const Akonadi::Item &item, items) {
        storeNote(item);
    }
    //kDebug() << "loaded" << mNotesLoaded << "notes; now hash has" << mNotesHash.count() << "entries";
    if (mNotesLoaded == mNotesCollection.statistics().count())
        emit notesLoaded(mNotesLoaded);
}

void NotesRepository::storeNote(const Akonadi::Item &item)
{
    if (item.hasPayload<SugarNote>()) {
        SugarNote note = item.payload<SugarNote>();
        const QString id = note.id();
        Q_ASSERT(!id.isEmpty());
        removeNote(id); // handle change of opp
        if (note.parentType() == QLatin1String("Opportunities")) {
            const QString parentId = note.parentId();
            if (!parentId.isEmpty()) {
                mNotesHash[parentId].append(note);
                mNotesParentIdHash.insert(id, parentId);
            } else {
                mNotesParentIdHash.remove(id);
            }
        } else {
            // We also get notes for Accounts and Emails.
            // (well, no longer, we filter this out in the resource)
            //kDebug() << "ignoring notes for" << note.parentType();
        }
    }
}

void NotesRepository::removeNote(const QString &id)
{
    Q_ASSERT(!id.isEmpty());
    const QString oldParentId = mNotesParentIdHash.value(id);
    if (!oldParentId.isEmpty()) {
        kDebug() << "note" << id << "oldParentId" << oldParentId;
        // Note is no longer associated with this opportunity
        QVector<SugarNote> &notes = mNotesHash[oldParentId];
        auto it = std::find_if(notes.begin(), notes.end(), [id](const SugarNote &n) { return n.id() == id; });
        if (it != notes.end()) {
            const int idx = std::distance(notes.begin(), it);
            kDebug() << "Removing note at" << idx;
            notes.remove(idx);
        }
    }
}

///

void NotesRepository::setEmailsCollection(const Akonadi::Collection &collection)
{
    mEmailsCollection = collection;
}

void NotesRepository::loadEmails()
{
    //kDebug() << "Loading" << mEmailsCollection.statistics().count() << "emails";

    // load emails
    Akonadi::ItemFetchJob *job = new Akonadi::ItemFetchJob(mEmailsCollection, this);
    configureItemFetchScope(job->fetchScope());
    connect(job, SIGNAL(itemsReceived(Akonadi::Item::List)),
            this, SLOT(slotEmailsReceived(Akonadi::Item::List)));
}

void NotesRepository::monitorChanges()
{
    mMonitor = new Akonadi::Monitor(this);
    mMonitor->setCollectionMonitored(mNotesCollection);
    mMonitor->setCollectionMonitored(mEmailsCollection);
    configureItemFetchScope(mMonitor->itemFetchScope());
    connect(mMonitor, SIGNAL(itemAdded(Akonadi::Item,Akonadi::Collection)),
            this, SLOT(slotItemAdded(Akonadi::Item,Akonadi::Collection)));
    connect(mMonitor, SIGNAL(itemRemoved(Akonadi::Item)),
            this, SLOT(slotItemRemoved(Akonadi::Item)));
    connect(mMonitor, SIGNAL(itemChanged(Akonadi::Item,QSet<QByteArray>)),
            this, SLOT(slotItemChanged(Akonadi::Item,QSet<QByteArray>)));
}

QVector<SugarEmail> NotesRepository::emailsForOpportunity(const QString &id) const
{
    return mEmailsHash.value(id);
}

void NotesRepository::slotEmailsReceived(const Akonadi::Item::List &items)
{
    mEmailsLoaded += items.count();
    foreach(const Akonadi::Item &item, items) {
        storeEmail(item);
    }
    //kDebug() << "loaded" << mEmailsLoaded << "emails; now hash has" << mEmailsHash.count() << "entries";
    if (mEmailsLoaded == mEmailsCollection.statistics().count())
        emit emailsLoaded(mEmailsLoaded);
}

void NotesRepository::storeEmail(const Akonadi::Item &item)
{
    if (item.hasPayload<SugarEmail>()) {
        SugarEmail email = item.payload<SugarEmail>();
        const QString id = email.id();
        Q_ASSERT(!id.isEmpty());
        removeEmail(id); // handle change of opp
        if (email.parentType() == QLatin1String("Opportunities")) {
            const QString parentId = email.parentId();
            if (!parentId.isEmpty()) {
                mEmailsHash[parentId].append(email);
                mEmailsParentIdHash.insert(id, parentId);
            } else {
                mEmailsParentIdHash.remove(id);
            }
        } else {
            // We also get emails for Accounts and Emails.
            // (well, no longer, we filter this out in the resource)
            //kDebug() << "ignoring emails for" << email.parentType();
        }
    }
}

void NotesRepository::removeEmail(const QString &id)
{
    Q_ASSERT(!id.isEmpty());
    const QString oldParentId = mEmailsParentIdHash.value(id);
    if (!oldParentId.isEmpty()) {
        kDebug() << "email" << id << "oldParentId" << oldParentId;
        // Email is no longer associated with this opportunity
        QVector<SugarEmail> &emails = mEmailsHash[oldParentId];
        auto it = std::find_if(emails.begin(), emails.end(), [id](const SugarEmail &n) { return n.id() == id; });
        if (it != emails.end()) {
            const int idx = std::distance(emails.begin(), it);
            kDebug() << "Removing email at" << idx;
            emails.remove(idx);
        }
    }
}

void NotesRepository::configureItemFetchScope(Akonadi::ItemFetchScope &scope)
{
    scope.setFetchRemoteIdentification(false);
    scope.setIgnoreRetrievalErrors(true);
    scope.fetchFullPayload(true);
}

void NotesRepository::updateItem(const Akonadi::Item &item, const Akonadi::Collection &collection)
{
    if (collection == mNotesCollection) {
        storeNote(item);
    } else if (collection == mEmailsCollection) {
        storeEmail(item);
    } else {
        kWarning() << "Unexpected collection" << collection << ", expected" << mNotesCollection.id() << "or" << mEmailsCollection.id();
    }
}

void NotesRepository::slotItemAdded(const Akonadi::Item &item, const Akonadi::Collection &collection)
{
    kDebug() << item.id() << item.mimeType();
    updateItem(item, collection);
}

void NotesRepository::slotItemRemoved(const Akonadi::Item &item)
{
    kDebug() << item.id();
    const Akonadi::Collection collection = item.parentCollection();
    // Don't use the payload to handle removals (no payload anymore on removal)
    if (collection == mNotesCollection) {
        removeNote(item.remoteId());
    } else if (collection == mEmailsCollection) {
        removeEmail(item.remoteId());
    } else {
        kWarning() << "Unexpected collection" << collection << ", expected" << mNotesCollection.id() << "or" << mEmailsCollection.id();
    }
}

void NotesRepository::slotItemChanged(const Akonadi::Item &item, const QSet<QByteArray> &partIdentifiers)
{
    // I get only REMOTEREVISION even when changing the parentid in the SugarEmail...
    //kDebug() << item.id() << partIdentifiers;
    Q_UNUSED(partIdentifiers);

    // Handle the case where the parent id changed
    const Akonadi::Collection collection = item.parentCollection();
    updateItem(item, collection);
}
