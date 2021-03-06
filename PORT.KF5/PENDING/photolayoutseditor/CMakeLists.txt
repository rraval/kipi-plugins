#
# Copyright (c) 2010-2017, Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

if (POLICY CMP0063)
    cmake_policy(SET CMP0063 NEW)
endif (POLICY CMP0063)

#add_subdirectory(extra/qtpropertybrowser)
add_subdirectory(resources/app)
add_subdirectory(data)

include_directories($<TARGET_PROPERTY:Qt5::Xml,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:Qt5::Gui,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:Qt5::Core,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:Qt5::Widgets,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:Qt5::PrintSupport,INTERFACE_INCLUDE_DIRECTORIES>

                    $<TARGET_PROPERTY:KF5::XmlGui,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:KF5::I18n,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:KF5::ConfigCore,INTERFACE_INCLUDE_DIRECTORIES>
)

set(photolayoutseditor_SRCS

    events/ProgressEvent.cpp
    events/UndoCommandEvent.cpp
    events/UndoCommandEventFilter.cpp

    models/AbstractMovableModel.cpp
    models/LayersModel.cpp
    models/LayersModelItem.cpp
    models/LayersSelectionModel.cpp

    borderplugins/StandardBordersFactory.cpp
    borderplugins/PolaroidBorderDrawer.cpp
    borderplugins/SolidBorderDrawer.cpp

    borders/BorderChangeListener.cpp
    borders/BorderDrawerInterface.cpp
    borders/BorderDrawersLoader.cpp
    borders/BordersGroup.cpp

    effectplugins/StandardEffectsFactory.cpp
    effectplugins/BlurPhotoEffect.cpp
    effectplugins/ColorizePhotoEffect.cpp
    effectplugins/GrayscalePhotoEffect.cpp
    effectplugins/SepiaPhotoEffect.cpp
    effectplugins/NegativePhotoEffect.cpp

    effects/AbstractPhotoEffectInterface.cpp
    effects/PhotoEffectChangeListener.cpp
    effects/PhotoEffectsGroup.cpp
    effects/PhotoEffectsLoader.cpp

    threads/AbstractPhotoItemLoader.cpp
    threads/CanvasLoadingThread.cpp
    threads/CanvasSavingThread.cpp
    threads/ImageLoadingThread.cpp
    threads/PhotoItemLoader.cpp
    threads/SceneBackgroundLoader.cpp
    threads/SceneBorderLoader.cpp
    threads/TextItemLoader.cpp

    widgets/dialogs/CanvasSizeDialog.cpp
    widgets/dialogs/GridSetupDialog.cpp
    widgets/dialogs/ImageFileDialog.cpp
    widgets/dialogs/NewCanvasDialog.cpp
    widgets/dialogs/CanvasSizeWidget.cpp
    widgets/dialogs/TemplatesModel.cpp
    widgets/dialogs/TemplatesView.cpp
    widgets/extra/PatternsComboBox.cpp
    widgets/extra/PLEStatusBar.cpp
    widgets/layers/LayersTree.cpp
    widgets/layers/LayersTreeDelegate.cpp
    widgets/items/AbstractPhoto.cpp
    widgets/items/AbstractPhoto_p.cpp
    widgets/items/PhotoItem.cpp
    widgets/items/TextItem.cpp
    widgets/items/AbstractItemInterface.cpp
    widgets/menus/LayersTreeMenu.cpp
    widgets/tools/AbstractItemsListViewTool.cpp
    widgets/tools/AbstractItemsTool.cpp
    widgets/tools/AbstractTool.cpp
    widgets/tools/BorderEditTool.cpp
    widgets/tools/CanvasEditTool.cpp
    widgets/tools/EffectsEditorTool.cpp
    widgets/tools/TextEditorTool.cpp
    widgets/tools/ToolsDockWidget.cpp
    widgets/tools/ZoomTool.cpp
    widgets/canvas/Canvas.cpp
    widgets/canvas/CanvasSize.cpp
    widgets/canvas/CropWidgetItem.cpp
    widgets/canvas/MousePressListener.cpp
    widgets/canvas/RotationWidgetItem.cpp
    widgets/canvas/ScalingWidgetItem.cpp
    widgets/canvas/Scene.cpp
    widgets/canvas/SceneBackground.cpp
    widgets/canvas/SceneBorder.cpp

    listeners/TextColorChangeListener.cpp
    listeners/TextFontChangeListener.cpp

    undo/UndoMoveRowsCommand.cpp

    settings/PLEConfigDialog.cpp
    settings/PLEConfigViewWidget.cpp

    extra/global.cpp
    extra/KEditFactory.cpp

    main/photolayoutswindow.cpp
   )

set(photolayoutseditor_RCS
    resources/icons.qrc
   )

qt5_add_resources(photolayoutseditor_SRCS ${photolayoutseditor_RCS})

#kconfig_add_kcfg_files(photolayoutseditor_SRCS GENERATE_MOC ./settings/PLEConfigSkeleton.kcfgc)

add_library(photolayoutseditor_src OBJECT ${photolayoutseditor_SRCS})

install(FILES ./settings/photolayoutseditor.kcfg DESTINATION ${KCFG_INSTALL_DIR})
install(PROGRAMS main/photolayoutseditor.desktop DESTINATION ${XDG_APPS_INSTALL_DIR})
install(FILES main/photolayoutseditorui.rc DESTINATION ${DATA_INSTALL_DIR}/photolayoutseditor)
install(FILES effects/photolayoutseditoreffectplugin.desktop DESTINATION ${SERVICETYPES_INSTALL_DIR})
install(FILES borders/photolayoutseditorborderplugin.desktop DESTINATION ${SERVICETYPES_INSTALL_DIR})

# =======================================================
# Stand-alone version of PhotoLayoutsEditor

#set(photolayoutseditor_SRCS main/main.cpp)

#file(GLOB ICONS_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/./resources/app/*photolayoutseditor.png")
#ecm_add_app_icon(photolayoutseditor_SRCS ICONS ${ICONS_SRCS})

#add_executable(photolayoutseditor ${photolayoutseditor_SRCS})

#target_link_libraries(photolayoutseditor libphotolayoutseditor)
