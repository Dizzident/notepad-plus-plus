// This file is part of Notepad++ project
// Copyright (C)2024 Notepad++ contributors

// This program is free software: you are free to redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// at your option any later version.

#pragma once

#include "../StaticDialog/StaticDialog.h"
#include <memory>

// Forward declarations
class QSlider;
class QLabel;
class QVBoxLayout;
class QHBoxLayout;
class QFrame;

// Forward declaration from ScintillaComponent
class ScintillaEditView;
class Buffer;
struct MapPosition;

namespace QtControls {

// ============================================================================
// ViewZoneWidget - Custom widget for drawing the visible region highlight
// ============================================================================
class ViewZoneWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ViewZoneWidget(QWidget* parent = nullptr);

    void setZone(int higherY, int lowerY);
    int getViewerHeight() const { return _lowerY - _higherY; }
    int getCurrentCenterPosY() const { return (_lowerY - _higherY) / 2 + _higherY; }

    void setFocusColor(const QColor& color) { _focusColor = color; }
    void setFrostColor(const QColor& color) { _frostColor = color; }

signals:
    void zoneClicked(int y);
    void zoneDragged(int y);

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;

private:
    int _higherY = 0;
    int _lowerY = 0;
    QColor _focusColor;
    QColor _frostColor;
    bool _dragging = false;
};


// ============================================================================
// DocumentMap - Qt implementation of Document Map panel
// ============================================================================
class DocumentMap : public StaticDialog {
    Q_OBJECT

public:
    DocumentMap(QWidget* parent = nullptr);
    ~DocumentMap() override;

    // Initialize with the main editor view
    void init(ScintillaEditView** ppEditView);

    // Show the dialog/panel
    void doDialog();

    // Event processing override
    bool run_dlgProc(QEvent* event) override;

    // Bring getDialog into public scope for getWidget
    using StaticDialog::getDialog;

    // Get widget for docking integration
    QWidget* getWidget() const { return getDialog(); }

    // Update the map view from the main editor
    void updateMap();

    // Set zoom level for the map view (-10 to 20, default -10)
    void setZoomLevel(int level);
    int getZoomLevel() const { return _zoomLevel; }

    // Reload the map (when document changes)
    void reloadMap();

    // Show a temporary buffer in the map
    void showInMapTemporarily(Buffer* buf2show, ScintillaEditView* fromEditView);

    // Wrap/unwrap the map
    void wrapMap(const ScintillaEditView* editView = nullptr);
    void initWrapMap();

    // Scroll operations
    void scrollMap();
    void scrollMap(bool direction, bool perPage);
    void scrollMapWith(const MapPosition& mapPos);

    // Folding support
    void fold(size_t line, bool foldOrNot);
    void foldAll(bool mode);

    // Syntax highlighting
    void setSyntaxHighlighting();

    // Text direction
    void changeTextDirection(bool isRTL);

    // Check if temporarily showing a different buffer
    bool isTemporarilyShowing() const { return _isTemporarilyShowing; }
    void setTemporarilyShowing(bool tempShowing) { _isTemporarilyShowing = tempShowing; }

    // Redraw the map
    void redrawMap(bool forceUpdate = false);

public slots:
    void onMapClicked(int y);
    void onMapScrolled();
    void onZoomChanged(int value);
    void onMainEditorScrolled();
    void onMainEditorChanged();

protected:
    void setupUI();
    void connectSignals();
    void resizeEvent(QResizeEvent* event) override;

private:
    void syncWithEditor();
    void highlightVisibleRegion();
    void scrollEditorToPosition(int pos);
    void doMove();
    bool needToRecomputeWith(const ScintillaEditView* editView = nullptr);

    // Scintilla views
    ScintillaEditView* _mapView = nullptr;        // The map's Scintilla view
    ScintillaEditView** _ppEditView = nullptr;    // Pointer to main editor view

    // UI Components
    QVBoxLayout* _mainLayout = nullptr;
    QFrame* _mapContainer = nullptr;
    ViewZoneWidget* _viewZone = nullptr;
    QSlider* _zoomSlider = nullptr;
    QLabel* _zoomLabel = nullptr;

    // State
    int _zoomLevel = -10;           // Default zoomed out
    bool _updating = false;         // Prevent recursive updates
    bool _isTemporarilyShowing = false;

    // For wrap computation
    intptr_t _displayZoom = -1;
    intptr_t _displayWidth = 0;

    // Colors for view zone
    static constexpr QColor DEFAULT_FOCUS_COLOR = QColor(0xFF, 0x80, 0x00);  // Orange
    static constexpr QColor DEFAULT_FROST_COLOR = QColor(0xFF, 0xFF, 0xFF); // White (transparent)
};

} // namespace QtControls
