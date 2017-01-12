/*
  This file is part of FatCRM, a desktop application for SugarCRM written by KDAB.

  Copyright (C) 2015-2016 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com
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

#ifndef ACCOUNTDETAILS_H
#define ACCOUNTDETAILS_H

#include "details.h"

namespace Ui
{
class AccountDetails;
}

class AccountDataExtractor;
class LinkedItemsRepository;

class AccountDetails : public Details
{
    Q_OBJECT
public:
    explicit AccountDetails(QWidget *parent = 0);

    ~AccountDetails() override;

    void setLinkedItemsRepository(LinkedItemsRepository *repo) Q_DECL_OVERRIDE { mLinkedItemsRepository = repo; }
    ItemDataExtractor *itemDataExtractor() const Q_DECL_OVERRIDE;

private Q_SLOTS:
    void slotVisitWebsite();
    void slotBillingAddressCountryEditingFinished();
    void slotShippingAddressCountryEditingFinished();
    void on_viewNotesButton_clicked();
    void on_viewDocumentsButton_clicked();

private:
    Ui::AccountDetails *mUi;

private:
    void initialize();
    QMap<QString, QString> data(const Akonadi::Item &item) const Q_DECL_OVERRIDE;
    void updateItem(Akonadi::Item &item, const QMap<QString, QString> &data) const Q_DECL_OVERRIDE;
    void setDataInternal(const QMap<QString, QString> &data) Q_DECL_OVERRIDE;
    AccountDataExtractor *mDataExtractor;
    LinkedItemsRepository *mLinkedItemsRepository;
};

#endif /* ACCOUNTDETAILS_H */

