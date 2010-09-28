/**
 * \file CheckboxDelegate.h
 * 19/08/2010 LB Initial implementation
 */

#ifndef CHECKBOXDELEGATE_H 
#define CHECKBOXDELEGATE_H 

#include <QItemDelegate>

class QWidget;
class QRect;

/**
 * CheckboxDelegate modifies a model view to display boolean values as checkboxes.
**/
class CheckboxDelegate : public QItemDelegate
{
	Q_OBJECT
	
public:
	CheckboxDelegate (int checkboxColumn, QObject* parent = 0);

	void paint(QPainter* painter, const QStyleOptionViewItem& option,
		const QModelIndex& index) const;
	QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem &option,
		const QModelIndex& index) const;
	void setEditorData(QWidget* editor, const QModelIndex& index) const;
	void setModelData(QWidget* editor, QAbstractItemModel* model,
		const QModelIndex& index) const;
	void updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option,
		const QModelIndex& index) const;
	bool editorEvent(QEvent *event, QAbstractItemModel *model,
		const QStyleOptionViewItem &option, const QModelIndex &index);

public slots:
	void test(int state);

private:
	int _checkboxColumn;

	QRect checkboxRect(const QStyleOptionViewItem& viewItemStyleOptions) const;
};

#endif // CHECKBOXDELEGATE_H
