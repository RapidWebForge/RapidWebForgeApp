    #ifndef TUTORIALSPANEL_H
    #define TUTORIALSPANEL_H

    #include <QDialog>
    #include <QGridLayout>
    #include <QLabel>
    #include <QPushButton>
    #include <QWidget>
    #include "../../core/configuration-manager/configurationmanager.h"
    #include "../../models/project/project.h"
    #include "../stepper/stepper.h"
    #include <vector>

    namespace Ui {
    class TutorialsPanel;
    }

    class TutorialsPanel : public QDialog
    {
        Q_OBJECT

    public:
        explicit TutorialsPanel(QWidget *parent = nullptr);
        ~TutorialsPanel();
        void setupTutorials();

    private:
        Ui::TutorialsPanel *ui;
        QGridLayout *gridLayout;
        std::vector<Project> projects;
        ConfigurationManager *confManager = nullptr;

    private slots:
        void onAddProjectClicked();
        void onProjectPreviewClicked(const QString &tutorialPath, int projectId);
        void onDeleteProjectRequested(int projectId);
        // 📌 Nuevo slot para manejar cuando se hace clic en un tutorial
        void onTutorialClicked(const QString &tutorialPath, int projectId);

    signals:
        void projectCreationRequested();
        void projectClicked(const Project &project);
        void deleteRequested(int projectId);
        void openTutorial(const QString &tutorialPath, int projectId);
    };

    #endif // TUTORIALSPANEL_H
