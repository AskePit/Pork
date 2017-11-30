/********************************************************************************
** Form generated from reading UI file 'videoplayerwidget.ui'
**
** Created by: Qt User Interface Compiler version 5.6.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_VIDEOPLAYERWIDGET_H
#define UI_VIDEOPLAYERWIDGET_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QSlider>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QWidget>
#include "videoplayer.h"

QT_BEGIN_NAMESPACE

class Ui_VideoPlayerWidget
{
public:
    QSlider *progressSlider;
    QSlider *volumeSlider;
    QLabel *codecErrorLabel;
    QGridLayout *videoLayout;
    QSpacerItem *topMargin;
    QSpacerItem *leftMargin;
    QSpacerItem *rightMargin;
    aske::VideoView *videoView;
    QSpacerItem *bottomMargin;

    void setupUi(QWidget *videoWidget)
    {
        if (videoWidget->objectName().isEmpty())
            videoWidget->setObjectName(QStringLiteral("videoWidget"));
        videoWidget->setMouseTracking(true);
        videoWidget->setStyleSheet(QStringLiteral("background-color:black;"));
        progressSlider = new QSlider(videoWidget);
        progressSlider->setObjectName(QStringLiteral("progressSlider"));
        progressSlider->setGeometry(QRect(0, 0, 84, 19));
        progressSlider->setFocusPolicy(Qt::NoFocus);
        progressSlider->setStyleSheet(QLatin1String("QSlider::groove:horizontal {\n"
"background: #eee;\n"
"height: 2px;\n"
"}\n"
"\n"
"QSlider::groove:horizontal:hover {\n"
"background: #eee;\n"
"height: 4px;\n"
"}\n"
"\n"
"QSlider::sub-page:horizontal {\n"
"background: white;\n"
"height: 2px;\n"
"}\n"
"\n"
"QSlider::add-page:horizontal {\n"
"background: #777;\n"
"height: 2px;\n"
"}\n"
"\n"
"QSlider::handle {\n"
"width: 0px;\n"
"height: 0px;\n"
"}"));
        progressSlider->setOrientation(Qt::Horizontal);
        volumeSlider = new QSlider(videoWidget);
        volumeSlider->setObjectName(QStringLiteral("volumeSlider"));
        volumeSlider->setGeometry(QRect(0, 0, 19, 84));
        volumeSlider->setFocusPolicy(Qt::NoFocus);
        volumeSlider->setStyleSheet(QLatin1String("QSlider::groove:vertical {\n"
"background: #eee;\n"
"width: 2px;\n"
"}\n"
"\n"
"QSlider::groove:vertical:hover {\n"
"background: #eee;\n"
"width:4px;\n"
"}\n"
"\n"
"QSlider::sub-page:vertical {\n"
"background: #777;\n"
"width: 2px;\n"
"}\n"
"\n"
"QSlider::add-page:vertical {\n"
"background: white;\n"
"width: 2px;\n"
"}\n"
"\n"
"QSlider::handle {\n"
"width: 0px;\n"
"height: 0px;\n"
"}"));
        volumeSlider->setOrientation(Qt::Vertical);
        codecErrorLabel = new QLabel(videoWidget);
        codecErrorLabel->setObjectName(QStringLiteral("codecErrorLabel"));
        codecErrorLabel->setGeometry(QRect(0, 0, 241, 51));
        QFont font;
        font.setPointSize(20);
        codecErrorLabel->setFont(font);
        codecErrorLabel->setAlignment(Qt::AlignCenter);
        videoLayout = new QGridLayout(videoWidget);
        videoLayout->setSpacing(6);
        videoLayout->setContentsMargins(11, 11, 11, 11);
        videoLayout->setObjectName(QStringLiteral("videoLayout"));
        topMargin = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        videoLayout->addItem(topMargin, 0, 1, 1, 1);

        leftMargin = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        videoLayout->addItem(leftMargin, 1, 0, 1, 1);

        rightMargin = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        videoLayout->addItem(rightMargin, 1, 2, 1, 1);

        videoView = new aske::VideoView(videoWidget);
        videoView->setObjectName(QStringLiteral("videoView"));
        QPalette palette;
        QBrush brush(QColor(0, 0, 0, 255));
        brush.setStyle(Qt::SolidPattern);
        palette.setBrush(QPalette::Active, QPalette::Button, brush);
        palette.setBrush(QPalette::Active, QPalette::Base, brush);
        palette.setBrush(QPalette::Active, QPalette::Window, brush);
        palette.setBrush(QPalette::Inactive, QPalette::Button, brush);
        palette.setBrush(QPalette::Inactive, QPalette::Base, brush);
        palette.setBrush(QPalette::Inactive, QPalette::Window, brush);
        palette.setBrush(QPalette::Disabled, QPalette::Button, brush);
        palette.setBrush(QPalette::Disabled, QPalette::Base, brush);
        palette.setBrush(QPalette::Disabled, QPalette::Window, brush);
        videoView->setPalette(palette);
        videoView->setMouseTracking(true);
        videoView->setStyleSheet(QStringLiteral(""));

        videoLayout->addWidget(videoView, 1, 1, 1, 1);

        bottomMargin = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        videoLayout->addItem(bottomMargin, 2, 1, 1, 1);


        retranslateUi(videoWidget);

        QMetaObject::connectSlotsByName(videoWidget);
    } // setupUi

    void retranslateUi(QWidget *videoWidget)
    {
        codecErrorLabel->setText(QApplication::translate("VideoPlayerWidget", "<html><head/><body><p><span style=\" color:#ffffff;\">Unknown codec!</span></p></body></html>", 0));
        Q_UNUSED(videoWidget);
    } // retranslateUi

};

namespace Ui {
    class VideoPlayerWidget: public Ui_VideoPlayerWidget {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_VIDEOPLAYERWIDGET_H
