#include "itemswidget.h"
#include "ui_itemswidget.h"

#include "EditorItemManager.h"
#include "API/Stats.h"

#include <QTextBlock>
#include <QMessageBox>
#include <QDir>

ItemsWidget::ItemsWidget(QScrollArea *parent) :
    QScrollArea(parent),
    ui(new Ui::ItemsWidget),
    items(NULL),
    itemsEdited(false),
    editedStyle("background-color:#66FF99;"),
    invalidStyle("background-color:#FF3333;")
{
    ui->setupUi(this);
}

ItemsWidget::~ItemsWidget()
{
    delete items;
}

void ItemsWidget::saveItems(const std::string &path)
{
    if (items != NULL) items->save(path);
    setItemsAreEdited(false);
}

void ItemsWidget::loadItems(const std::string &path)
{
    items = new EditorItemManager(path);
    for (unsigned i = 0; i<items->items.size(); i++)
    {
        if (items->items[i].name != "")
        {
            QListWidgetItem* item = new QListWidgetItem(qString(items->items[i].name));
            item->setData(Qt::UserRole, i);
            ui->itemsList->addItem(item);

        }
    }
    ui->itemTypeCB->clear();
    std::map<std::string, std::string>::iterator iter;
    for (iter = items->item_types.begin(); iter != items->item_types.end(); ++iter)
    {
        ui->itemTypeCB->addItem(qString(iter->second), qString(iter->first));
    }
    checkComboBoxForError(ui->itemTypeCB, "items/types.txt is missing. Copy it from base mod.");

    for (unsigned i = 0; i<items->equip_flags.size(); i++)
    {
        ui->equipList->addItem(qString(items->equip_flags[i]));
    }
    checkComboBoxForError(ui->equipList, "engine/equip_flags.txt is missing. Copy it from base mod.");

    for (unsigned i = 0; i<items->slot_type.size(); i++)
    {
        ui->slotsList->addItem(qString(items->slot_type[i]));
    }
    checkComboBoxForError(ui->slotsList, "menus/inventory.txt is missing. Copy it from base mod.");

    for (unsigned i = 0; i<items->elements.size(); i++)
    {
        ui->bonusList->addItem(qString(items->elements[i]) + "_resist");
    }
    checkComboBoxForError(ui->bonusList, "engine/elements.txt is missing. Copy it from base mod.");

    ui->bonusList->addItem("speed");
    ui->bonusList->addItem("physical");
    ui->bonusList->addItem("mental");
    ui->bonusList->addItem("offense");
    ui->bonusList->addItem("defense");
    for (unsigned i = 0; i<STAT_COUNT; i++)
    {
        ui->bonusList->addItem(qString(STAT_KEY[i]));
    }

    ui->classList->addItem("");
    for (unsigned i = 0; i<items->hero_classes.size(); i++)
    {
        ui->classList->addItem(qString(items->hero_classes[i]));
    }
    ui->pushBtn->setEnabled(false);

    collectFileLists(path);
    on_clearBtn_clicked();
}

void ItemsWidget::clearItemsList()
{
    ui->itemsList->clear();
    setItemsAreEdited(false);
    delete items;
    items = NULL;
}

bool ItemsWidget::itemsAreEdited()
{
    return itemsEdited;
}

void ItemsWidget::setItemsAreEdited(bool state)
{
    if (state)
        emit itemsWereEdited();
    else
        emit itemsNotEdited();
    itemsEdited = state;
}

void ItemsWidget::on_addNewItem_clicked()
{
	items->items.resize(items->items.size() + 1);
    int index = items->items.size() - 1;
    items->items[index].name = "newItem";

    QListWidgetItem* item = new QListWidgetItem("newItem");
    item->setData(Qt::UserRole, index);
    ui->itemsList->addItem(item);
}

void ItemsWidget::on_clearBtn_clicked()
{
    ui->itemName->clear();
	ui->itemName->setText("ItemName");
    ui->itemFlavor->clear();
    ui->itemBook->clear();
    ui->classList->clear();
	ui->pickupStatus->clear();
	ui->powerDesc->clear();
    ui->replacePowerFrom->clear();
    ui->replacePowerTo->clear();
    ui->disableSlots->clear();
    ui->equipFlags->clear();
    ui->bonusName->clear();
    ui->bonusValue->clear();

    // comboBoxes
    ui->itemQualityCB->setCurrentIndex(1);
    ui->itemTypeCB->setCurrentIndex(-1);
    ui->sfxCb->setCurrentIndex(-1);
    ui->lootAnimList->setCurrentIndex(-1);
    ui->stepSoundList->setCurrentIndex(-1);
    ui->equipAnimList->setCurrentIndex(-1);

    // spinBoxes
    ui->itemLvlSpin->setValue(0);
    ui->price->setValue(0);
    ui->sellPrice->setValue(0);
    ui->absorbMax->setValue(0);
    ui->absorbMin->setValue(0);
    ui->maxQuantity->setValue(0);
    ui->meleeMin->setValue(0);
    ui->meleeMax->setValue(0);
    ui->mentalMin->setValue(0);
    ui->mentalMax->setValue(0);
    ui->power->setValue(0);
    ui->rangMin->setValue(0);
    ui->rangMax->setValue(0);
    ui->reqDef->setValue(0);
    ui->reqMent->setValue(0);
    ui->reqOff->setValue(0);
    ui->reqDef->setValue(0);
}

void ItemsWidget::on_pushBtn_clicked()
{
    int index = ui->itemsList->currentItem()->data(Qt::UserRole).toInt();

    // TextEdits
    items->items[index].name = stdString(ui->itemName->text());
    items->items[index].flavor = stdString(ui->itemFlavor->text());
    items->items[index].pickup_status = stdString(ui->pickupStatus->text());
    items->items[index].power_desc = stdString(ui->powerDesc->text());
    items->items[index].book = stdString(ui->itemBook->text());

    QTextDocument* docFrom = ui->replacePowerFrom->document();
    QTextDocument* docTo   = ui->replacePowerTo->document();
    items->items[index].replace_power.clear();

    for (int i = 0; i < docFrom->lineCount(); i++)
    {
        if (docFrom->findBlockByLineNumber(i).text().isEmpty() || docTo->findBlockByLineNumber(i).text().isEmpty())
            continue;
        items->items[index].replace_power.push_back(Point(
                                                        docFrom->findBlockByLineNumber(i).text().toInt(),
                                                        docTo->findBlockByLineNumber(i).text().toInt()));
    }

    QTextDocument* disabledSlots = ui->disableSlots->document();
    items->items[index].disable_slots.clear();

    for (int i = 0; i < disabledSlots->lineCount(); i++)
    {
        if (disabledSlots->findBlockByLineNumber(i).text().isEmpty())
            continue;
        items->items[index].disable_slots.push_back(stdString(disabledSlots->findBlockByLineNumber(i).text()));
    }

    QTextDocument* equipFlags = ui->equipFlags->document();
    items->items[index].equip_flags.clear();

    for (int i = 0; i < equipFlags->lineCount(); i++)
    {
        if (equipFlags->findBlockByLineNumber(i).text().isEmpty())
            continue;
        items->items[index].equip_flags.push_back(stdString(equipFlags->findBlockByLineNumber(i).text()));
    }

    QTextDocument* bonusName    = ui->bonusName->document();
    QTextDocument* bonusValue   = ui->bonusValue->document();
    items->items[index].bonus.clear();

    for (int i = 0; i < bonusName->lineCount(); i++)
    {
        if (bonusName->findBlockByLineNumber(i).text().isEmpty() || bonusValue->findBlockByLineNumber(i).text().isEmpty())
            continue;
        items->items[index].bonus.push_back(BonusData());

        QString bonus_str = bonusName->findBlockByLineNumber(i).text();

        if (bonus_str == "speed") {
            items->items[index].bonus.back().is_speed = true;
        }
        else if (bonus_str == "physical") {
            items->items[index].bonus.back().base_index = 0;
        }
        else if (bonus_str == "mental") {
            items->items[index].bonus.back().base_index = 1;
        }
        else if (bonus_str == "offense") {
            items->items[index].bonus.back().base_index = 2;
        }
        else if (bonus_str == "defense") {
            items->items[index].bonus.back().base_index = 3;
        }

        for (unsigned k=0; k<items->elements.size(); ++k) {
            if (bonus_str == qString(items->elements[k]) + "_resist")
            {
                items->items[index].bonus.back().resist_index = k;
                break;
            }
        }
        for (unsigned k=0; k<STAT_COUNT; ++k) {
            if (bonus_str == qString(STAT_KEY[k])) {
                items->items[index].bonus.back().stat_index = (STAT)k;
                break;
            }
        }

        items->items[index].bonus.back().value = bonusValue->findBlockByLineNumber(i).text().toInt();
    }

    // comboBoxes
    items->items[index].type     = stdString(ui->itemTypeCB->itemData(ui->itemTypeCB->currentIndex()).toString());
    items->items[index].quality  = ui->itemQualityCB->currentIndex();
    items->items[index].requires_class = stdString(ui->classList->currentText());

    items->items[index].loot_animation.resize(1);
    items->items[index].loot_animation.back().name = std::string("animations/loot/") + stdString(ui->lootAnimList->itemText(ui->lootAnimList->currentIndex()));

    items->items[index].sfx    = std::string("soundfx/inventory/") + stdString(ui->sfxCb->itemText(ui->sfxCb->currentIndex()));
    items->items[index].gfx    = stdString(ui->equipAnimList->itemText(ui->equipAnimList->currentIndex()));
    items->items[index].stepfx = stdString(ui->stepSoundList->itemText(ui->stepSoundList->currentIndex()));

    // spinBoxes
    items->items[index].level        = ui->itemLvlSpin->value();
    items->items[index].price        = ui->price->value();
    items->items[index].price_sell   = ui->sellPrice->value();
    items->items[index].abs_max      = ui->absorbMax->value();
    items->items[index].abs_min      = ui->absorbMin->value();
    items->items[index].max_quantity = ui->maxQuantity->value();
    items->items[index].dmg_melee_min  = ui->meleeMin->value();
    items->items[index].dmg_melee_max  = ui->meleeMax->value();
    items->items[index].dmg_ment_max   = ui->mentalMin->value();
    items->items[index].dmg_ment_max   = ui->mentalMax->value();
    items->items[index].dmg_ranged_max = ui->rangMin->value();
    items->items[index].dmg_ranged_min = ui->rangMax->value();
    items->items[index].power          = ui->power->value();

    items->items[index].req_stat.clear();
    items->items[index].req_val.clear();

    if (ui->reqPhys->value() > 0)
    {
        items->items[index].req_stat.push_back(REQUIRES_PHYS);
        items->items[index].req_val.push_back(ui->reqPhys->value());
    }
    if (ui->reqMent->value() > 0)
    {
        items->items[index].req_stat.push_back(REQUIRES_MENT);
        items->items[index].req_val.push_back(ui->reqMent->value());
    }
    if (ui->reqOff->value() > 0)
    {
        items->items[index].req_stat.push_back(REQUIRES_OFF);
        items->items[index].req_val.push_back(ui->reqOff->value());
    }
    if (ui->reqDef->value() > 0)
    {
        items->items[index].req_stat.push_back(REQUIRES_DEF);
        items->items[index].req_val.push_back(ui->reqDef->value());
    }

    //Update ListBox
    ui->itemsList->currentItem()->setData(Qt::DisplayRole, ui->itemName->text());

    setItemsAreEdited(true);
}

void ItemsWidget::on_itemsList_itemClicked(QListWidgetItem *item)
{
    ui->pushBtn->setEnabled(true);
    int index = item->data(Qt::UserRole).toInt();

    // TextEdits
    ui->itemName->setText(qString(items->items[index].name));
    ui->pickupStatus->setText(qString(items->items[index].pickup_status));
    ui->powerDesc->setText(qString(items->items[index].power_desc));
    ui->itemFlavor->setText(qString(items->items[index].flavor));
    ui->itemBook->setText(qString(items->items[index].book));

    ui->replacePowerFrom->clear();
    ui->replacePowerTo->clear();
    for (unsigned int i = 0; i < items->items[index].replace_power.size(); i++)
    {
        ui->replacePowerFrom->appendPlainText(QString::number(items->items[index].replace_power[i].x));
        ui->replacePowerTo->appendPlainText(QString::number(items->items[index].replace_power[i].y));
    }

    ui->disableSlots->clear();
    for (unsigned int i = 0; i < items->items[index].disable_slots.size(); i++)
    {
        ui->disableSlots->appendPlainText(qString(items->items[index].disable_slots[i]));
    }

    ui->equipFlags->clear();
    for (unsigned int i = 0; i < items->items[index].equip_flags.size(); i++)
    {
        ui->equipFlags->appendPlainText(qString(items->items[index].equip_flags[i]));
    }

    ui->bonusName->clear();
    ui->bonusValue->clear();
    for (unsigned int i = 0; i < items->items[index].bonus.size(); i++)
    {
        int stat_index     = items->items[index].bonus[i].stat_index;
        int base_index     = items->items[index].bonus[i].base_index;
        int resist_index   = items->items[index].bonus[i].resist_index;
        bool is_speed      = items->items[index].bonus[i].is_speed;

        if (stat_index != -1)
        {
            ui->bonusName->appendPlainText(qString(STAT_KEY[stat_index]));
        }
        else if (resist_index != -1)
        {
            ui->bonusName->appendPlainText(qString(items->elements[resist_index]) + "_resist");
        }
        else if (is_speed)
        {
            ui->bonusName->appendPlainText(QString("speed"));
        }
        else if (base_index != -1)
        {
            QString bonus_str;
            if (base_index == 0)
                bonus_str = "physical";
            else if (base_index == 1)
               bonus_str = "mental";
            else if (base_index == 2)
                bonus_str = "offense";
            else if (base_index == 3)
                bonus_str = "defense";

            ui->bonusName->appendPlainText(bonus_str);
        }
        ui->bonusValue->appendPlainText(QString::number(items->items[index].bonus[i].value));
    }

    // comboBoxes
    QString type = qString(items->items[index].type);
    ui->itemTypeCB->setCurrentIndex(-1);
    for (int i = 0; i < ui->itemTypeCB->count(); i++)
    {
        if (ui->itemTypeCB->itemData(i) == type)
        {
            ui->itemTypeCB->setCurrentIndex(i);
            break;
        }
    }
    ui->itemQualityCB->setCurrentIndex(items->items[index].quality);

    type = qString(items->items[index].requires_class);
    selectComboBoxItemByText(ui->classList, type);

    QString soundfx = qString(items->items[index].sfx);
    selectComboBoxItemByText(ui->sfxCb, QFileInfo(soundfx).fileName());

    QString stepfx = qString(items->items[index].stepfx);
    selectComboBoxItemByText(ui->stepSoundList, stepfx);

    QString loot_anim;
    if (!items->items[index].loot_animation.empty())
    {
        loot_anim = qString(items->items[index].loot_animation.back().name);
        selectComboBoxItemByText(ui->lootAnimList, QFileInfo(loot_anim).fileName());
    }

    QString gfx = qString(items->items[index].gfx);
    selectComboBoxItemByText(ui->equipAnimList, gfx);

    // spinBoxes
    ui->itemLvlSpin->setValue(items->items[index].level);
    ui->price->setValue(items->items[index].price);
    ui->sellPrice->setValue(items->items[index].price_sell);
    ui->absorbMax->setValue(items->items[index].abs_max);
    ui->absorbMin->setValue(items->items[index].abs_min);
    ui->maxQuantity->setValue(items->items[index].max_quantity);
    ui->meleeMin->setValue(items->items[index].dmg_melee_min);
    ui->meleeMax->setValue(items->items[index].dmg_melee_max);
    ui->mentalMin->setValue(items->items[index].dmg_ment_max);
    ui->mentalMax->setValue(items->items[index].dmg_ment_max);
    ui->rangMin->setValue(items->items[index].dmg_ranged_max);
    ui->rangMax->setValue(items->items[index].dmg_ranged_min);
    ui->power->setValue(items->items[index].power);

    ui->reqPhys->setValue(0);
    ui->reqMent->setValue(0);
    ui->reqOff->setValue(0);
    ui->reqDef->setValue(0);
    for (unsigned int i = 0; i < items->items[index].req_stat.size(); i++)
    {
        int value = items->items[index].req_val[i];

        if (items->items[index].req_stat[i] == REQUIRES_PHYS)
            ui->reqPhys->setValue(value);

        else if (items->items[index].req_stat[i] == REQUIRES_MENT)
            ui->reqMent->setValue(value);

        else if (items->items[index].req_stat[i] == REQUIRES_OFF)
            ui->reqOff->setValue(value);

        else if (items->items[index].req_stat[i] == REQUIRES_DEF)
            ui->reqDef->setValue(value);
    }
}

void ItemsWidget::on_absorbMin_valueChanged(int arg1)
{
    markNotDefaultSpinBox(ui->absorbMin, arg1, 0);
}

void ItemsWidget::on_absorbMax_valueChanged(int arg1)
{
    markNotDefaultSpinBox(ui->absorbMax, arg1, 0);
}

void ItemsWidget::on_power_valueChanged(int arg1)
{
    markNotDefaultSpinBox(ui->power, arg1, 0);
}

void ItemsWidget::on_itemFlavor_textChanged(const QString &arg1)
{
    markNotEmptyLineEdit(ui->itemFlavor, arg1);
}

void ItemsWidget::on_itemBook_textChanged(const QString &arg1)
{
    markNotEmptyLineEdit(ui->itemBook, arg1);
}

void ItemsWidget::on_itemQualityCB_currentIndexChanged(const QString &arg1)
{
    if (arg1 != "normal")
    {
        ui->itemQualityCB->setStyleSheet(editedStyle);
    }
    else
    {
        ui->itemQualityCB->setStyleSheet("");
    }
}

void ItemsWidget::on_meleeMin_valueChanged(int arg1)
{
    markNotDefaultSpinBox(ui->meleeMin, arg1, 0);
}

void ItemsWidget::on_meleeMax_valueChanged(int arg1)
{
    markNotDefaultSpinBox(ui->meleeMax, arg1, 0);
}

void ItemsWidget::on_rangMin_valueChanged(int arg1)
{
    markNotDefaultSpinBox(ui->rangMin, arg1, 0);
}

void ItemsWidget::on_rangMax_valueChanged(int arg1)
{
    markNotDefaultSpinBox(ui->rangMax, arg1, 0);
}

void ItemsWidget::on_mentalMin_valueChanged(int arg1)
{
    markNotDefaultSpinBox(ui->mentalMin, arg1, 0);
}

void ItemsWidget::on_mentalMax_valueChanged(int arg1)
{
    markNotDefaultSpinBox(ui->mentalMax, arg1, 0);
}

void ItemsWidget::on_replacePowerFrom_textChanged()
{
    markNotEmptyPlainTextEdit(ui->replacePowerFrom);
}

void ItemsWidget::on_replacePowerTo_textChanged()
{
    markNotEmptyPlainTextEdit(ui->replacePowerTo);
}

void ItemsWidget::on_disableSlots_textChanged()
{
    markNotEmptyPlainTextEdit(ui->disableSlots);
}

void ItemsWidget::on_reqPhys_valueChanged(int arg1)
{
    markNotDefaultSpinBox(ui->reqPhys, arg1, 0);
}

void ItemsWidget::on_reqMent_valueChanged(int arg1)
{
    markNotDefaultSpinBox(ui->reqMent, arg1, 0);
}

void ItemsWidget::on_reqOff_valueChanged(int arg1)
{
    markNotDefaultSpinBox(ui->reqOff, arg1, 0);
}

void ItemsWidget::on_reqDef_valueChanged(int arg1)
{
    markNotDefaultSpinBox(ui->reqDef, arg1, 0);
}

void ItemsWidget::on_price_valueChanged(int arg1)
{
    markNotDefaultSpinBox(ui->price, arg1, 0);
}

void ItemsWidget::on_sellPrice_valueChanged(int arg1)
{
    markNotDefaultSpinBox(ui->sellPrice, arg1, 0);
}

void ItemsWidget::on_maxQuantity_valueChanged(int arg1)
{
    markNotDefaultSpinBox(ui->maxQuantity, arg1, 1);
}

void ItemsWidget::on_pickupStatus_textChanged(const QString &arg1)
{
    markNotEmptyLineEdit(ui->pickupStatus, arg1);
}

void ItemsWidget::on_powerDesc_textChanged(const QString &arg1)
{
    markNotEmptyLineEdit(ui->powerDesc, arg1);
}

void ItemsWidget::on_itemName_textChanged(const QString &arg1)
{
    if (arg1 != "")
    {
        ui->itemName->setStyleSheet("");
        ui->itemName->setToolTip("");
    }
    else
    {
        ui->itemName->setStyleSheet(invalidStyle);
        ui->itemName->setToolTip("Item name should be not empty");
    }
}

void ItemsWidget::on_equipFlags_textChanged()
{
    markNotEmptyPlainTextEdit(ui->equipFlags);
}

void ItemsWidget::on_bonusName_textChanged()
{
    markNotEmptyPlainTextEdit(ui->bonusName);
}

void ItemsWidget::on_bonusValue_textChanged()
{
    markNotEmptyPlainTextEdit(ui->bonusValue);
}

void ItemsWidget::on_addDisableSlot_clicked()
{
    ui->disableSlots->appendPlainText(ui->slotsList->currentText());
}

void ItemsWidget::on_addEquipFlag_clicked()
{
    ui->equipFlags->appendPlainText(ui->equipList->currentText());
}

void ItemsWidget::on_addBonus_clicked()
{
    ui->bonusName->appendPlainText(ui->bonusList->currentText());
}

void ItemsWidget::collectFileLists(const std::string &path)
{
    QString modPath = qString(path);
    QDir pathSfx(modPath + "soundfx" + QDir::separator() + "inventory");
    QStringList files = pathSfx.entryList(QDir::Files);
    ui->sfxCb->addItems(files);
    checkComboBoxForError(ui->sfxCb, "soundfx/inventory folder is empty. Place some sound files in it.");

    QDir pathLootAnim(modPath + "animations" + QDir::separator() + "loot");
    files = pathLootAnim.entryList(QDir::Files);
    ui->lootAnimList->addItems(files);
    checkComboBoxForError(ui->lootAnimList, "animations/loot folder is empty. Place some loot animation files in it.");

    QDir pathStepFx(modPath + "soundfx" + QDir::separator() + "steps");
    files = pathStepFx.entryList(QDir::Files);
    for (int i = 0; i < files.size(); i++)
    {
        files[i].remove(0, 5);
        files[i].remove(files[i].size() - 5, 5);
    }
    files.removeDuplicates();
    ui->stepSoundList->addItems(files);
    checkComboBoxForError(ui->stepSoundList, "soundfx/steps folder is empty. Place some sound files in it.");

    QDir pathGfx(modPath + "animations" + QDir::separator() + "avatar" + QDir::separator() + "male");
    files = pathGfx.entryList(QDir::Files);
    for (int i = 0; i < files.size(); i++)
    {
        files[i].remove(files[i].size() - 4, 4);
    }
    ui->equipAnimList->addItems(files);
    checkComboBoxForError(ui->equipAnimList, "animations/avatar/male folder is empty. Place some equip animation files in it.");
}

QString ItemsWidget::qString(std::string value)
{

    return QString::fromUtf8(value.data(), value.size());
}

std::string ItemsWidget::stdString(QString value)
{
    return value.toUtf8().constData();
}

void ItemsWidget::checkComboBoxForError(QComboBox *widget, const QString &errorText)
{
    if (widget->count() == 0)
    {
        widget->setStyleSheet(invalidStyle);
        widget->setToolTip(errorText);
    }
    else
    {
        widget->setStyleSheet("");
        widget->setToolTip("");
    }
}

void ItemsWidget::markNotEmptyLineEdit(QLineEdit *widget, const QString& text)
{
    if (text != "")
    {
        widget->setStyleSheet(editedStyle);
    }
    else
    {
        widget->setStyleSheet("");
    }
}

void ItemsWidget::markNotEmptyPlainTextEdit(QPlainTextEdit *widget)
{
    QTextDocument* doc = widget->document();

    if (doc->lineCount() >= 1 && !doc->findBlockByLineNumber(0).text().isEmpty())
    {
        widget->setStyleSheet(editedStyle);
    }
    else
    {
        widget->setStyleSheet("");
    }
}

void ItemsWidget::selectComboBoxItemByText(QComboBox *widget, const QString &text)
{
    widget->setCurrentIndex(-1);
    for (int i = 0; i < widget->count(); i++)
    {
        if (widget->itemText(i) == text)
        {
            widget->setCurrentIndex(i);
            break;
        }
    }
}

void ItemsWidget::markNotDefaultSpinBox(QSpinBox *widget, int value, int defaultValue)
{
    if (value != defaultValue)
    {
        widget->setStyleSheet(editedStyle);
    }
    else
    {
        widget->setStyleSheet("");
    }
}
