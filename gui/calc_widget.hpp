#ifndef CALCWIDGET_HPP
#define CALCWIDGET_HPP

// #include <iostream>
// #include <fstream>

#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QDoubleSpinBox>
#include <QtWidgets/QRadioButton>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QProgressBar>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QButtonGroup>
#include <QtConcurrent/QtConcurrentRun>
#include <QtCore/QTextStream>

#include "constants.hpp"
#include "calculation.hpp"

namespace GUI {

class CalcWidget : public QWidget {
    Q_OBJECT

    public:
        CalcWidget(const Arguments& args);

    signals:
        void StartingCalculation();
        void OverwriteFile();

    public slots:
        void Go();
        void ChangeOutput(QTextStream* newOutStream);
        void GiveOverwriteWarnings(const bool newValue);

    private:
        void SetupInputBoxes(QLayout* layout, const Arguments& args);
        QFrame* SetupOneLevelFrame(const Arguments& args);
        QFrame* SetupAllLevelsFrame(const Arguments& args);
        void SetupParameterBoxes(QLayout* layout, const Arguments& args);
        void SetupButtons(QLayout* layout);

        enum OverwriteWarning { WARNING_ON  = 0b10, WARNING_ACTIVE = 0b01 };
        void Calculate();

        QTextStream* console;
        QTextStream* outStream;
        int warningStatus;
        // number fields for N, L, P; I think that's the following 4 things:
        QSpinBox* nBox;
        QSpinBox* lBox;
        QDoubleSpinBox* dBox;
        QSpinBox* kMaxBox;
        QDoubleSpinBox* msqBox;
        QDoubleSpinBox* lambdaBox;
        QDoubleSpinBox* cutoffBox;
        QRadioButton* freeButton;
        QRadioButton* interactingButton;
        QRadioButton* testButton;
        QPushButton* goButton;
        QProgressBar* progressBar;
};

}

#endif
