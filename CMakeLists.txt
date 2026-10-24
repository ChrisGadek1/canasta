cmake_minimum_required(VERSION 3.5)
project(canasta VERSION 1.0 LANGUAGES C CXX)

set(CMAKE_GENERATOR_PLATFORM Win32)
set(CMAKE_PREFIX_PATH "D:\\QT_5.6.3\\5.6.3\\mingw49_32")
set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt5 5.6.3 REQUIRED COMPONENTS Core Gui Widgets OpenGL)

qt5_add_resources(RESOURCES
    resources.qrc
)

add_executable(canasta MACOSX_BUNDLE
    aiplayer.cpp aiplayer.h
    card.cpp card.h
    cardmemory.cpp cardmemory.h
    cardsdeck.cpp cardsdeck.h
    cardsdecktype.h
    cardsuit.h
    cardvalue.h
    game.cpp game.h
    main.cpp
    mainglwidget.cpp mainglwidget.h
    mainwindow.cpp mainwindow.h mainwindow.ui
    meld.cpp meld.h
    player.cpp player.h
    position.cpp position.h
    roundresult.h
    teamroundscore.h
    ${RESOURCES}
)

if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/results.json)
    configure_file(results.json ${CMAKE_CURRENT_BINARY_DIR}/results.json COPYONLY)
endif()

target_link_libraries(canasta PRIVATE
    Qt5::Core
    Qt5::Gui
    Qt5::Widgets
    Qt5::OpenGL
    opengl32
)


# Resources:
set(resources_resource_files
    "images/background.png"
    "images/cards/10_of_clubs.png"
    "images/cards/10_of_diamonds.png"
    "images/cards/10_of_hearts.png"
    "images/cards/10_of_spades.png"
    "images/cards/2_of_clubs.png"
    "images/cards/2_of_diamonds.png"
    "images/cards/2_of_hearts.png"
    "images/cards/2_of_spades.png"
    "images/cards/3_of_clubs.png"
    "images/cards/3_of_diamonds.png"
    "images/cards/3_of_hearts.png"
    "images/cards/3_of_spades.png"
    "images/cards/4_of_clubs.png"
    "images/cards/4_of_diamonds.png"
    "images/cards/4_of_hearts.png"
    "images/cards/4_of_spades.png"
    "images/cards/5_of_clubs.png"
    "images/cards/5_of_diamonds.png"
    "images/cards/5_of_hearts.png"
    "images/cards/5_of_spades.png"
    "images/cards/6_of_clubs.png"
    "images/cards/6_of_diamonds.png"
    "images/cards/6_of_hearts.png"
    "images/cards/6_of_spades.png"
    "images/cards/7_of_clubs.png"
    "images/cards/7_of_diamonds.png"
    "images/cards/7_of_hearts.png"
    "images/cards/7_of_spades.png"
    "images/cards/8_of_clubs.png"
    "images/cards/8_of_diamonds.png"
    "images/cards/8_of_hearts.png"
    "images/cards/8_of_spades.png"
    "images/cards/9_of_clubs.png"
    "images/cards/9_of_diamonds.png"
    "images/cards/9_of_hearts.png"
    "images/cards/9_of_spades.png"
    "images/cards/ace_of_clubs.png"
    "images/cards/ace_of_diamonds.png"
    "images/cards/ace_of_hearts.png"
    "images/cards/ace_of_spades.png"
    "images/cards/ace_of_spades2.png"
    "images/cards/black_joker.png"
    "images/cards/card_reverse.png"
    "images/cards/jack_of_clubs.png"
    "images/cards/jack_of_clubs2.png"
    "images/cards/jack_of_diamonds.png"
    "images/cards/jack_of_diamonds2.png"
    "images/cards/jack_of_hearts.png"
    "images/cards/jack_of_hearts2.png"
    "images/cards/jack_of_spades.png"
    "images/cards/jack_of_spades2.png"
    "images/cards/king_of_clubs.png"
    "images/cards/king_of_clubs2.png"
    "images/cards/king_of_diamonds.png"
    "images/cards/king_of_diamonds2.png"
    "images/cards/king_of_hearts.png"
    "images/cards/king_of_hearts2.png"
    "images/cards/king_of_spades.png"
    "images/cards/king_of_spades2.png"
    "images/cards/queen_of_clubs.png"
    "images/cards/queen_of_clubs2.png"
    "images/cards/queen_of_diamonds.png"
    "images/cards/queen_of_diamonds2.png"
    "images/cards/queen_of_hearts.png"
    "images/cards/queen_of_hearts2.png"
    "images/cards/queen_of_spades.png"
    "images/cards/queen_of_spades2.png"
    "images/cards/red_joker.png"
)

target_sources(canasta PRIVATE ${resources_resource_files})


install(TARGETS canasta
    BUNDLE DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# Consider using qt_generate_deploy_app_script() for app deployment if
# the project can use Qt 6.3. In that case rerun qmake2cmake with
# --min-qt-version=6.3.
