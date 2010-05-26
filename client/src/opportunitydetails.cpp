#include "opportunitydetails.h"

#include <akonadi/item.h>

#include <kdcrmdata/sugaropportunity.h>

using namespace Akonadi;


OpportunityDetails::OpportunityDetails( QWidget *parent )
    : QWidget( parent )

{
    mUi.setupUi( this );
    initialize();
}

OpportunityDetails::~OpportunityDetails()
{
    delete mCalendarButton;
}

void OpportunityDetails::initialize()
{

    QList<QLineEdit*> lineEdits =  mUi.opportunityInformationGB->findChildren<QLineEdit*>();
    Q_FOREACH( QLineEdit* le, lineEdits )
        connect( le, SIGNAL( textChanged( const QString& ) ),
                 this, SLOT( slotEnableSaving() ) );

    QList<QComboBox*> comboBoxes =  mUi.opportunityInformationGB->findChildren<QComboBox*>();
    Q_FOREACH( QComboBox* cb, comboBoxes )
        connect( cb, SIGNAL( currentIndexChanged( int ) ),
                 this, SLOT( slotEnableSaving() ) );

     QList<QGroupBox*> groupBoxes =
        mUi.opportunityInformationGB->findChildren<QGroupBox*>();
    Q_FOREACH( QGroupBox* gb, groupBoxes ) {
       connect( gb, SIGNAL( toggled( bool ) ),
                this, SLOT( slotEnableSaving() ) );
       connect( gb, SIGNAL( toggled( bool ) ),
                this, SLOT( slotSetModifyFlag( bool ) ) );
    }

    mModifyFlag = false;

    mCalendarButton = new EditCalendarButton(this);
    QVBoxLayout *buttonLayout = new QVBoxLayout;
    buttonLayout->addWidget( mCalendarButton );
    mUi.calendarWidget->setLayout( buttonLayout );

    connect( mCalendarButton->calendarWidget(), SIGNAL( selectionChanged() ),
             this, SLOT( slotSetCloseDate() ) );

    connect( mUi.description, SIGNAL( textChanged() ),
             this,  SLOT( slotEnableSaving() ) );

    connect( mUi.saveButton, SIGNAL( clicked() ),
             this, SLOT( slotSaveOpportunity() ) );

    mUi.saveButton->setEnabled( false );
}

void OpportunityDetails::setItem (const Item &item )
{
    // opportunity info
    const SugarOpportunity opportunity = item.payload<SugarOpportunity>();
    mUi.name->setText( opportunity.name() );
    mUi.accountName->setCurrentIndex( mUi.accountName->findText( opportunity.accountName() ) );
    mUi.opportunityType->setCurrentIndex( mUi.opportunityType->findText( opportunity.opportunityType() ) );
    mUi.leadSource->setCurrentIndex( mUi.leadSource->findText( opportunity.leadSource() ) );
    mUi.campaignName->setCurrentIndex(mUi.campaignName->findText( opportunity.campaignName() ) );
    mUi.salesStage->setCurrentIndex(mUi.salesStage->findText( opportunity.salesStage() ) );
    mUi.assignedUserName->setCurrentIndex(mUi.assignedUserName->findText( opportunity.assignedUserName() ) );
    mUi.opportunityType->setCurrentIndex(mUi.opportunityType->findText( opportunity.opportunityType() ) );
    mUi.assignedUserName->setCurrentIndex(mUi.assignedUserName->findText( opportunity.assignedUserName() ) );
    // Pending (michel) - currency todo
    mUi.amount->setText( opportunity.amount() );
    // Pending (michel) - closed date todo
    mUi.nextStep->setText( opportunity.nextStep() );
    mUi.probability->setText( opportunity.probability() );
}

void OpportunityDetails::clearFields ()
{
    // reset line edits
    QList<QLineEdit*> lineEdits =
        mUi.opportunityInformationGB->findChildren<QLineEdit*>();
    Q_FOREACH( QLineEdit* le, lineEdits )
        le->setText(QString());

    // reset label and properties
    QList<QLabel*> labels =
        mUi.opportunityInformationGB->findChildren<QLabel*>();
    Q_FOREACH( QLabel* lab, labels ) {
        QString value = lab->objectName();
        if ( value == "modifiedBy" ) {
            lab->clear();
            lab->setProperty( "modifiedUserId", qVariantFromValue<QString>( QString() ) );
        }
        else if ( value == "createdDate" ) {
            lab->clear();
            lab->setProperty( "id", qVariantFromValue<QString>( QString() ) );
            lab->setProperty( "deleted", qVariantFromValue<QString>( QString() ) );
        }
        else if ( value == "createdBy" ) {
            lab->clear();
            lab->setProperty( "createdBy", qVariantFromValue<QString>( QString() ) );
        }
    }

    // enable
    QList<QGroupBox*> groupBoxes =
        mUi.opportunityInformationGB->findChildren<QGroupBox*>();
    Q_FOREACH( QGroupBox* gb, groupBoxes )
        gb->setChecked( true );

    // reset combos
    QList<QComboBox*> comboBoxes =
        mUi.opportunityInformationGB->findChildren<QComboBox*>();
    Q_FOREACH( QComboBox* cb, comboBoxes )
        cb->setCurrentIndex( 0 );

    // initialize other fields
    mUi.description->clear();
    mUi.name->setFocus();


    // we are creating a new contact
    slotSetModifyFlag( false );

}


void OpportunityDetails::slotSetModifyFlag( bool value )
{
    mModifyFlag = value;
}

void OpportunityDetails::slotEnableSaving()
{
    QList<QGroupBox*> groupBoxes =
        mUi.opportunityInformationGB->findChildren<QGroupBox*>();

    Q_FOREACH( QGroupBox* gb, groupBoxes )
        if ( gb->isChecked() ) {
            mUi.saveButton->setEnabled( true );
            return;
        }
    mUi.saveButton->setEnabled( false );
}

void OpportunityDetails::slotSaveOpportunity()
{
    if ( !mData.empty() )
        mData.clear();

    mUi.modifiedDate->setText( QDateTime::currentDateTime().toString( QString( "yyyy-MM-dd hh:mm:ss") ) );

    QList<QLineEdit*> lineEdits =
        mUi.opportunityInformationGB->findChildren<QLineEdit*>();
    Q_FOREACH( QLineEdit* le, lineEdits )
        mData[le->objectName()] = le->text();

    QList<QLabel*> labels =
        mUi.opportunityInformationGB->findChildren<QLabel*>();
    Q_FOREACH( QLabel* lab, labels ) {
        QString objName = lab->objectName();
        if ( objName == "modifiedDate" )
            mData["modifiedDate"] = lab->text();
        else if ( objName == "modifiedBy" ) {
            mData["modifiedBy"] = lab->text();
            mData["modifiedUserId"] = lab->property( "modifiedUserId" ).toString();
            mData["modifiedUserName"] = lab->property( "modifiedUserName" ).toString();
        }
        else if ( objName == "createdDate" ) {
            mData["createdDate"] = lab->text();
            mData["id"] = lab->property( "id" ).toString();
            mData["deleted"] = lab->property( "deleted" ).toString();
        }
        else if ( objName == "createdBy" ) {
            mData["createdByName"] = lab->text();
            mData["createdBy"] = lab->property( "createdBy" ).toString();
        }
    }

    mData["opportunityType"] = mUi.opportunityType->currentText();

    mData["campaignName"] = mUi.campaignName->currentText();
    mData["campaignId"] = mCampaignsData.value( mUi.campaignName->currentText() );

    mData["assignedUserName"] = mUi.assignedUserName->currentText();
    mData["assignedUserId"] = mAssignedToData.value( mUi.assignedUserName->currentText() );
    mData["description"] = mUi.description->toPlainText();

    if ( !mModifyFlag )
        emit saveOpportunity();
    else
        emit modifyOpportunity();

}

void OpportunityDetails::addAccountData( const QString &accountName,  const QString &accountId )
{
    mAccountsData.insert( accountName, accountId );
}

void OpportunityDetails::addCampaignData( const QString &campaignName,  const QString &campaignId )
{
    mCampaignsData.insert( campaignName, campaignId );
}

void OpportunityDetails::addAssignedToData( const QString &name, const QString &id )
{
    mAssignedToData.insert( name, id );
}

void OpportunityDetails::fillCombos()
{

    QList<QString> names = mAccountsData.uniqueKeys();
    // fill
    // accountName combo
    for ( int i = 0; i < names.count(); ++i )
        mUi.accountName->addItem( names[i] );
    // campaign
    names = mCampaignsData.uniqueKeys();
    for ( int i = 0; i < names.count(); ++i )
        mUi.campaignName->addItem( names[i] );
    // assigned to
    names = mAssignedToData.uniqueKeys();
    for ( int i = 0; i < names.count(); ++i )
        mUi.assignedUserName->addItem( names[i] );
}

void OpportunityDetails::disableGroupBoxes()
{
    QList<QGroupBox*> groupBoxes =
        mUi.opportunityInformationGB->findChildren<QGroupBox*>();

    Q_FOREACH( QGroupBox* gb, groupBoxes )
        gb->setChecked( false );
}


void OpportunityDetails::slotSetDateClosed()
{
    mUi.dateClosed->setText( mCalendarButton->calendarWidget()->selectedDate().toString( QString("yyyy-MM-dd" ) ) );
}
