/* ============================================================
 *
 * Date        : 2010-06-01
 * Description : A widget to search for places
 *
 * Copyright (C) 2010 by Michael G. Hansen <mike at mghansen dot de>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#ifndef SEARCHWIDGET_H
#define SEARCHWIDGET_H

// Qt includes

#include <QAbstractItemModel>
#include <QWidget>

// local includes

#include "searchbackend.h"

namespace KIPIGPSSyncPlugin
{

class SearchResultItem;
class SearchResultModelPrivate;
class SearchResultModel : public QAbstractItemModel
{
Q_OBJECT

public:
    class SearchResultItem
    {
    public:
        SearchBackend::SearchResult result;
    };

    SearchResultModel(QObject* const parent = 0);
    ~SearchResultModel();

    // QAbstractItemModel:
    virtual int columnCount(const QModelIndex& parent = QModelIndex() ) const;
    virtual bool setData(const QModelIndex& index, const QVariant& value, int role);
    virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;
    virtual QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex() ) const;
    virtual QModelIndex parent(const QModelIndex& index) const;
    virtual int rowCount(const QModelIndex& parent = QModelIndex()) const;
    virtual bool setHeaderData(int section, Qt::Orientation orientation, const QVariant& value, int role);
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    virtual Qt::ItemFlags flags(const QModelIndex& index) const;

    void addResults(const SearchBackend::SearchResult::List& results);
    SearchResultItem resultItem(const QModelIndex& index) const;

private:
    SearchResultModelPrivate* const d;
};

class SearchResultModelHelperPrivate;
class SearchResultModelHelper : public WMW2::WMWModelHelper
{
Q_OBJECT
public:
    SearchResultModelHelper(SearchResultModel* const resultModel, QObject* const parent = 0);
    ~SearchResultModelHelper();

    virtual QAbstractItemModel* model() const;
    virtual QItemSelectionModel* selectionModel() const;
    virtual bool itemCoordinates(const QModelIndex& index, WMW2::WMWGeoCoordinate* const coordinates) const;
    virtual QPixmap itemIcon(const QModelIndex& index, QPoint* const offset) const;
    virtual bool visible() const;
    virtual bool snaps() const;

private:
    SearchResultModelHelperPrivate* const d;
};

class SearchWidgetPrivate;
class SearchWidget : public QWidget
{
Q_OBJECT

public:
    SearchWidget(QWidget* parent = 0);
    ~SearchWidget();

    WMW2::WMWModelHelper* getModelHelper();

private Q_SLOTS:
    void slotSearchCompleted();
    void slotTriggerSearch();
    void slotUpdateUIState();

private:
    SearchWidgetPrivate* const d;
};

} /* KIPIGPSSyncPlugin */

#endif /* SEARCHWIDGET_H */
