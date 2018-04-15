#include "file_widget.hpp"

namespace GUI {

FileWidget::FileWidget(): outStream(nullptr), outPath(new QLineEdit), 
    dontSave(new QCheckBox), suppressOverwriteWarning(new QCheckBox), 
    appendContents(new QCheckBox) {
    QVBoxLayout* layout = new QVBoxLayout;
    setLayout(layout);

    outPath->setText(tr("output file"));
    dontSave->setText(tr("don't save output"));
    suppressOverwriteWarning->setText(tr("don't warn about overwriting files"));
    appendContents->setText(tr("append to file contents (don't overwrite)"));

    layout->addWidget(outPath);
    layout->addWidget(dontSave);
    layout->addWidget(suppressOverwriteWarning);
    layout->addWidget(appendContents);

    // FIXME: need validator for outPath
    QRegExp pathRegex("\\w+\\.txt");
    QRegExpValidator* pathValidator = new QRegExpValidator(pathRegex);
    outPath->setValidator(pathValidator);

    connect(outPath, &QLineEdit::textChanged, 
            this, &FileWidget::ChangeOutputFileName);

    connect(dontSave, &QCheckBox::stateChanged, 
            this, &FileWidget::ChangeOutputStream);
    connect(appendContents, &QCheckBox::stateChanged, 
            this, &FileWidget::ChangeOutputStream);

    connect(dontSave, &QCheckBox::stateChanged,
            [=](bool checked){outPath->setEnabled(!checked);});
}

// this is for when the FILE needs to be changed; it's not called if only the
// stream is different
void FileWidget::ChangeOutputFileName() {
    if (!outPath->hasAcceptableInput()) return;

    QFileInfo fileInfo(outPath->text().toLocal8Bit());
    if (fileInfo.exists()) {
        if (fileInfo.isFile() && fileInfo.isWritable()) {
            QMessageBox confirm;
            confirm.setText("This file already exists.");
            confirm.setInformativeText("Do you want to overwrite it?");
            confirm.setStandardButtons(QMessageBox::Yes | QMessageBox::Cancel);
            confirm.setDefaultButton(QMessageBox::Cancel);
            if (confirm.exec() == QMessageBox::Yes) {
                ChangeOutputStream();
            } else {
                return;
            }
        } else {
            QMessageBox badName;
            badName.setText("This name already exists and is not a writable "
                    "file.");
            badName.exec();
            outPath->setText("output path");
            return;
        }
    } else {
        // file doesn't exist, go ahead and make it
        ChangeOutputStream();
    }
}

// is is called if the stream needs to change for any reason, such as a new
// filename or different write mode
//
// if this function is called, we assume that the contents of *outPath form a 
// valid file name
void FileWidget::ChangeOutputStream() {
    if (!(dontSave->isChecked() || outPath->hasAcceptableInput())) return;

    if (dontSave->isChecked()) {
        // if (outStream != nullptr) {
        if (outStream.rdbuf() != std::cout.rdbuf()) {
            std::cout << "outStream set to std::cout" << std::endl;
            outStream->close();
            outStream->rdbuf(std::cout.rdbuf());
            // emit OutputChanged(outStream.get());
            // outStream->close();
            // outStream = nullptr;
            // emit OutputChanged(&std::cout);
        }
        return;
    }

    auto writeMode = std::ios_base::out;
    if (appendContents->isChecked()) {
        writeMode |= std::ios_base::app;
    } else {
        writeMode |= std::ios_base::trunc;
    }
    std::cout << "outStream set to \"" << 
        (std::string)outPath->text().toLocal8Bit() << "\"" << std::endl;
    outStream->close();
    outStream->open(outPath->text().toLocal8Bit(), writeMode);
    // std::unique_ptr<std::ofstream> newOutStream = std::make_unique<std::ofstream>(
                                                // outPath->text().toLocal8Bit(), 
                                                // writeMode);
    // emit OutputChanged(newOutStream.get());

    // if (outStream != nullptr) outStream->close();
    // outStream = std::move(newOutStream);
}

} // namespace GUI