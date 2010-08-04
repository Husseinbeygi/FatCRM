
def main():
    startApplication("sugarclient")
    # load an existing account 
    clickTab(waitForObject(":SugarCRM Client: admin@SugarCRM on localhost.qt_tabwidget_tabbar_QTabBar"), "Leads")
    mouseClick(waitForObject(":Form.searchLE_QLineEdit_3"), 128, 10, 0, Qt.LeftButton)
    type(waitForObject(":Form.searchLE_QLineEdit_3"), "test account")
    clickButton(waitForObject(":SugarCRM Client: admin@SugarCRM on localhost.Show Details_QCheckBox"))
    # modify a few values
    mouseClick(waitForObject(":Details.campaignName_QComboBox_3"), 59, 10, 0, Qt.LeftButton)
    mouseClick(waitForObjectItem(":Details.campaignName_QComboBox_3", "test campaign"), 36, 7, 0, Qt.LeftButton)
    mouseClick(waitForObjectItem(":Details.salutation_QComboBox", "Mr\\."), 46, 6, 0, Qt.LeftButton)
    mouseDrag(waitForObject(":Details.firstName_QLineEdit"), 20, 14, 46, 0, 1, Qt.LeftButton)
    type(waitForObject(":Details.firstName_QLineEdit"), "Pigasse")
    mouseClick(waitForObject(":Other Details.status_QComboBox"), 97, 16, 0, Qt.LeftButton)
    mouseClick(waitForObjectItem(":Other Details.status_QComboBox", "Converted"), 90, 9, 0, Qt.LeftButton)
    mouseDrag(waitForObject(":Description:.description_QTextEdit_3"), 2, 11, 157, 5, 1, Qt.LeftButton)
    type(waitForObject(":Description:.description_QTextEdit_3"), "converted status")
    # save 
    clickButton(waitForObject(":&Lead Details.Save_QPushButton")) 
    activateItem(waitForObjectItem(":SugarCRM Client: admin@SugarCRM on localhost.menubar_QMenuBar", "File"))
    activateItem(waitForObjectItem(":SugarCRM Client: admin@SugarCRM on localhost.File_QMenu", "Syncronize"))
    label = waitForObject(':Lead Information.createdDate_QLabel')
    waitFor("label.text !=''")
    # data is back from remote client check modified values
    waitFor("object.exists(':Details.campaignName_QComboBox_3')")
    test.compare(findObject(":Details.campaignName_QComboBox_3").currentText, "test campaign")
    waitFor("object.exists(':Details.salutation_QComboBox')")
    test.compare(findObject(":Details.salutation_QComboBox").currentText, "Mr.")
    waitFor("object.exists(':Details.firstName_QLineEdit')")
    test.compare(findObject(":Details.firstName_QLineEdit").text, "asse")
    waitFor("object.exists(':Other Details.status_QComboBox')")
    test.compare(findObject(":Other Details.status_QComboBox").currentText, "Converted")
    waitFor("object.exists(':Description:.description_QTextEdit_3')")
    test.compare(findObject(":Description:.description_QTextEdit_3").plainText, "converted status")
    clickButton(waitForObject(":Form.New Lead_QPushButton"))
    clickButton(waitForObject(":Form.Clear_QToolButton_3"))
    mouseClick(waitForObject(":Form.searchLE_QLineEdit_3"), 329, 10, 0, Qt.LeftButton)
    type(waitForObject(":Form.searchLE_QLineEdit_3"), "test acc")
    waitForObjectItem(":Form.leadsTV_Akonadi::EntityTreeView", "max")
    clickItem(":Form.leadsTV_Akonadi::EntityTreeView", "max", 15, 5, 0, Qt.LeftButton)
    activateItem(waitForObjectItem(":SugarCRM Client: admin@SugarCRM on localhost.menubar_QMenuBar", "File"))
    activateItem(waitForObjectItem(":SugarCRM Client: admin@SugarCRM on localhost.File_QMenu", "Quit"))