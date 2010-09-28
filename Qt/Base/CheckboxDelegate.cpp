/**
 * \file CheckboxDelegate.cpp
 * 19/08/2010 LB Initial implementation
 * 
 * Implementation of CheckboxDelegate class
 */

// ** INCLUDES **
#include "CheckboxDelegate.h"
#include <QCheckBox>
#include <QPainter>
#include <QEvent>
#include <QApplication>
#include <QStyleOptionButton>
#include <QMouseEvent>

#include <iostream>

CheckboxDelegate::CheckboxDelegate(int checkboxColumn, QObject* parent)
	: QItemDelegate(parent)
{
	this->_checkboxColumn = checkboxColumn;
}

void CheckboxDelegate::test(int state)
{
	std::cout << "Test " << state << std::endl;
}

void CheckboxDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option,
	const QModelIndex& index) const
{
	if(index.isValid() && index.column() == _checkboxColumn)
	{
		bool checked = index.model()->data(index, Qt::DisplayRole).toBool();

		QStyleOptionButton check_box_style_option;
		check_box_style_option.state |= QStyle::State_Enabled;
		if (checked) {
			check_box_style_option.state |= QStyle::State_On;
		} else {
			check_box_style_option.state |= QStyle::State_Off;
		}
		check_box_style_option.rect = this->checkboxRect(option);

		QApplication::style()->drawControl(QStyle::CE_CheckBox,
									 &check_box_style_option,
									 painter);

	}
	else
		QItemDelegate::paint(painter, option, index);
}


QWidget* CheckboxDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem &option,
	const QModelIndex& index) const
{
	if(index.isValid() && index.column() == _checkboxColumn)
	{
		QCheckBox* checkbox = new QCheckBox(parent);
		//checkbox->installEventFilter(const_cast<CheckboxDelegate*>(this));
		//checkbox->show();
		checkbox->autoFillBackground();
		connect(checkbox, SIGNAL(stateChanged(int)), this, SLOT(test(int)));
		return checkbox;
	}
	else
	{
		return QItemDelegate::createEditor(parent, option, index);
	}
}

void CheckboxDelegate::setEditorData(QWidget* editor, const QModelIndex& index) const
{
	if(index.isValid() && index.column() == _checkboxColumn)
	{
		bool value = index.model()->data(index, Qt::DisplayRole).toBool();
		QCheckBox* checkbox = static_cast<QCheckBox*>(editor);
		if (value)
			checkbox->setCheckState(Qt::Checked);
		else
			checkbox->setCheckState(Qt::Unchecked);
	}
	else
		QItemDelegate::setEditorData(editor, index);
}

void CheckboxDelegate::setModelData(QWidget* editor, QAbstractItemModel* model,
	const QModelIndex& index) const
{
	if(index.isValid() && index.column() == _checkboxColumn)
	{
		bool value;
		QCheckBox* checkbox = static_cast<QCheckBox*>(editor);
		if (checkbox->checkState() == Qt::Checked)
			value = true;
		else
			value = false;
		
		model->setData(index, value);
	}
	else
		QItemDelegate::setModelData(editor, model, index);
}

void CheckboxDelegate::updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option,
	const QModelIndex& index) const
{
	if(index.isValid() && index.column() == _checkboxColumn)
		editor->setGeometry(option.rect);
	else
		QItemDelegate::updateEditorGeometry(editor, option, index);
}

bool CheckboxDelegate::editorEvent(QEvent *event, QAbstractItemModel *model,
	const QStyleOptionViewItem &option, const QModelIndex &index)
{
	Q_UNUSED(option);

	if ((event->type() == QEvent::MouseButtonRelease) ||
		  (event->type() == QEvent::MouseButtonDblClick)) {
		QMouseEvent *mouse_event = static_cast<QMouseEvent*>(event);
		if (mouse_event->button() != Qt::LeftButton ||
			!checkboxRect(option).contains(mouse_event->pos())) {
		  return false;
		}
		if (event->type() == QEvent::MouseButtonDblClick) {
		  return true;
		}
	  } else if (event->type() == QEvent::KeyPress) {
		if (static_cast<QKeyEvent*>(event)->key() != Qt::Key_Space &&
			static_cast<QKeyEvent*>(event)->key() != Qt::Key_Select) {
		  return false;
		}
	  } else {
		return false;
	  }

	  bool checked = index.model()->data(index, Qt::DisplayRole).toBool();
	  return model->setData(index, !checked, Qt::EditRole);
}

QRect CheckboxDelegate::checkboxRect(const QStyleOptionViewItem& viewItemStyleOptions) const
{
	QStyleOptionButton styleOptionButton;
	  QRect rect = QApplication::style()->subElementRect(
		  QStyle::SE_CheckBoxIndicator,
		  &styleOptionButton);
	  QPoint point(viewItemStyleOptions.rect.x() +
							 viewItemStyleOptions.rect.width() / 2 -
							 rect.width() / 2,
							 viewItemStyleOptions.rect.y() +
							 viewItemStyleOptions.rect.height() / 2 -
							 rect.height() / 2);
	  return QRect(point, rect.size());
}
