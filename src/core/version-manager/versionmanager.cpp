#include "versionmanager.h"
#include <QDir>
#include <QFileInfo>
#include <QMessageBox>
#include <QString>
#include <algorithm>
#include <ctime>
#include <fstream>
#include <git2.h>
#include <iostream>
#include <string>
#include <vector>

VersionManager::VersionManager(const std::string &projectPath)
    : projectPath(projectPath)
{
    static bool libgitInitialized = false;
    if (!libgitInitialized) {
        git_libgit2_init();
        libgitInitialized = true;
    }
}

void VersionManager::initializeRepository()
{
    git_repository *repo = nullptr;
    int error = git_repository_init(&repo, projectPath.c_str(), false);

    if (error == 0 && repo) {
        std::cout << "Git repository initialized successfully in: " << projectPath << std::endl;

        if (createGitignore()) {
            std::cout << "✅ .gitignore file created successfully.\n";
        } else {
            std::cerr << "❌ Failed to create .gitignore file.\n";
        }
    } else {
        const git_error *e = git_error_last();
        std::cerr << "❌ Failed to initialize Git repository: "
                  << (e ? e->message : "Unknown error") << std::endl;
    }

    if (repo) {
        git_repository_free(repo);
    }
}

bool VersionManager::createGitignore() const
{
    std::string gitignorePath = projectPath + "/.gitignore";
    std::ofstream gitignoreFile(gitignorePath);

    if (!gitignoreFile.is_open()) {
        return false;
    }

    gitignoreFile << "*node_modules/\n"
                  << "logs/\n"
                  << "temp/\n"
                  << "nginx.conf\n"
                  << "runEditor.js\n";
    gitignoreFile.close();
    return true;
}

void VersionManager::createVersion(const std::string &branchName)
{
    git_repository *repo = nullptr;
    git_index *index = nullptr;
    git_oid commit_oid;
    git_signature *sig = nullptr;
    git_status_list *status = nullptr;
    git_reference *branch_ref = nullptr;
    git_object *head_commit = nullptr;

    // Abrir repositorio
    if (git_repository_open(&repo, projectPath.c_str()) != 0) {
        std::cerr << "Error abriendo el repositorio" << std::endl;
        return;
    }

    // Obtener el índice
    if (git_repository_index(&index, repo) != 0) {
        std::cerr << "Error obteniendo el índice" << std::endl;
        git_repository_free(repo);
        return;
    }

    // Crear la firma
    if (git_signature_now(&sig, "Your Name", "your.email@example.com") != 0) {
        std::cerr << "Error creando la firma" << std::endl;
        git_index_free(index);
        git_repository_free(repo);
        return;
    }

    // Comprobar si hay cambios sin guardar
    git_status_options statusopt = GIT_STATUS_OPTIONS_INIT;
    statusopt.show = GIT_STATUS_SHOW_INDEX_AND_WORKDIR;
    statusopt.flags = GIT_STATUS_OPT_INCLUDE_UNTRACKED | GIT_STATUS_OPT_RECURSE_UNTRACKED_DIRS
                      | GIT_STATUS_OPT_DISABLE_PATHSPEC_MATCH;

    if (git_status_list_new(&status, repo, &statusopt) != 0) {
        std::cerr << "Error obteniendo el status" << std::endl;
        git_signature_free(sig);
        git_index_free(index);
        git_repository_free(repo);
        return;
    }

    size_t changes = git_status_list_entrycount(status);

    if (changes > 0) {
        // Hay cambios sin guardar, hacer commit
        git_oid tree_oid;
        git_tree *tree = nullptr;

        if (git_index_write_tree(&tree_oid, index) != 0) {
            std::cerr << "Error escribiendo el árbol" << std::endl;
            git_status_list_free(status);
            git_signature_free(sig);
            git_index_free(index);
            git_repository_free(repo);
            return;
        }

        if (git_index_write(index) != 0) {
            std::cerr << "Error escribiendo el índice" << std::endl;
            git_status_list_free(status);
            git_signature_free(sig);
            git_index_free(index);
            git_repository_free(repo);
            return;
        }

        if (git_tree_lookup(&tree, repo, &tree_oid) != 0) {
            std::cerr << "Error buscando el árbol" << std::endl;
            git_status_list_free(status);
            git_signature_free(sig);
            git_index_free(index);
            git_repository_free(repo);
            return;
        }

        if (git_revparse_single(&head_commit, repo, "HEAD") != 0) {
            // Puede fallar si es un repositorio vacío, no pasa nada
            head_commit = nullptr;
        }

        git_commit *parents[] = {(git_commit *) head_commit};
        size_t parent_count = head_commit ? 1 : 0;

        if (git_commit_create(&commit_oid,
                              repo,
                              "HEAD",
                              sig,
                              sig,
                              nullptr,
                              "Commit automático antes de crear rama",
                              tree,
                              parent_count,
                              parent_count > 0 ? parents : nullptr)
            != 0) {
            std::cerr << "Error creando el commit automático" << std::endl;
            git_tree_free(tree);
            git_status_list_free(status);
            git_signature_free(sig);
            git_index_free(index);
            git_repository_free(repo);
            return;
        }
        git_tree_free(tree);
    }

    // Crear la nueva rama desde HEAD
    if (git_revparse_single(&head_commit, repo, "HEAD") != 0) {
        std::cerr << "Error obteniendo HEAD para crear la rama" << std::endl;
        git_status_list_free(status);
        git_signature_free(sig);
        git_index_free(index);
        git_repository_free(repo);
        return;
    }

    if (git_branch_create(&branch_ref, repo, branchName.c_str(), (git_commit *) head_commit, 0)
        != 0) {
        std::cerr << "Error creando la nueva rama" << std::endl;
        git_object_free(head_commit);
        git_status_list_free(status);
        git_signature_free(sig);
        git_index_free(index);
        git_repository_free(repo);
        return;
    }

    // Cambiar a la nueva rama
    if (git_repository_set_head(repo, ("refs/heads/" + branchName).c_str()) != 0) {
        std::cerr << "Error cambiando HEAD a la nueva rama" << std::endl;
    }

    if (git_checkout_head(repo, nullptr) != 0) {
        std::cerr << "Error haciendo checkout a la nueva rama" << std::endl;
    }

    // Liberar recursos
    git_object_free(head_commit);
    git_reference_free(branch_ref);
    git_status_list_free(status);
    git_signature_free(sig);
    git_index_free(index);
    git_repository_free(repo);

    std::cout << "Rama '" << branchName << "' creada exitosamente." << std::endl;
}

void VersionManager::deleteVersion(const std::string &versionName)
{
    git_repository *repo = nullptr;
    git_reference *branchRef = nullptr;

    // Abrir el repositorio
    int error = git_repository_open(&repo, projectPath.c_str());
    if (error != 0) {
        std::cerr << "Failed to open repository: " << git_error_last()->message << std::endl;
        return;
    }

    // Buscar la rama por su nombre
    error = git_branch_lookup(&branchRef, repo, versionName.c_str(), GIT_BRANCH_LOCAL);
    if (error != 0) {
        std::cerr << "Failed to find branch: " << git_error_last()->message << std::endl;
        git_repository_free(repo);
        return;
    }

    // Borrar la rama
    error = git_branch_delete(branchRef);
    if (error == 0) {
        std::cout << "Branch '" << versionName << "' deleted successfully." << std::endl;
        // Confirmación de éxito
        QMessageBox::information(nullptr,
                                 "Delete Version",
                                 "Version '" + QString::fromStdString(versionName)
                                     + "' deleted successfully.");
    } else {
        std::cerr << "Failed to delete branch: " << git_error_last()->message << std::endl;
        // Error
        QMessageBox::critical(nullptr, "Delete Version", git_error_last()->message);
    }

    git_reference_free(branchRef);
    git_repository_free(repo);
}

void VersionManager::changeVersion(const std::string &versionName)
{
    git_repository *repo = nullptr;
    git_object *target = nullptr;

    // Abrir el repositorio
    int error = git_repository_open(&repo, projectPath.c_str());
    if (error != 0) {
        std::cerr << "Failed to open repository: " << git_error_last()->message << std::endl;
        return;
    }

    // Buscar la rama por su nombre
    error = git_revparse_single(&target, repo, ("refs/heads/" + versionName).c_str());
    if (error != 0) {
        std::cerr << "Failed to find branch: " << git_error_last()->message << std::endl;
        git_repository_free(repo);
        return;
    }

    // Hacer checkout de la rama
    error = git_checkout_tree(repo, target, nullptr);
    if (error == 0) {
        git_repository_set_head(repo, ("refs/heads/" + versionName).c_str());
        std::cout << "Switched to branch '" << versionName << "' successfully." << std::endl;
        // Confirmación de éxito
        QMessageBox::information(nullptr,
                                 "Change Version",
                                 "Switched to version: " + QString::fromStdString(versionName));
    } else {
        std::cerr << "Failed to switch to branch: " << git_error_last()->message << std::endl;
        // Error
        QMessageBox::critical(nullptr, "Change Version", git_error_last()->message);
    }

    git_object_free(target);
    git_repository_free(repo);
}

void VersionManager::saveChanges()
{
    git_repository *repo = nullptr;
    git_index *index = nullptr;
    git_oid treeOid, commitOid;
    git_tree *tree = nullptr;
    git_signature *sig = nullptr;
    git_status_list *status = nullptr;
    git_object *parent_commit = nullptr;

    // Abrir el repositorio
    int error = git_repository_open(&repo, projectPath.c_str());
    if (error != 0) {
        std::cerr << "Failed to open repository: " << git_error_last()->message << std::endl;
        return;
    }

    // Obtener el índice
    error = git_repository_index(&index, repo);
    if (error != 0) {
        std::cerr << "Failed to get repository index: " << git_error_last()->message << std::endl;
        git_repository_free(repo);
        return;
    }

    // Verificar si hay cambios pendientes
    git_status_options statusopt = GIT_STATUS_OPTIONS_INIT;
    statusopt.show = GIT_STATUS_SHOW_INDEX_AND_WORKDIR;
    statusopt.flags = GIT_STATUS_OPT_INCLUDE_UNTRACKED | GIT_STATUS_OPT_RECURSE_UNTRACKED_DIRS
                      | GIT_STATUS_OPT_DISABLE_PATHSPEC_MATCH;

    if (git_status_list_new(&status, repo, &statusopt) != 0) {
        std::cerr << "Failed to get status list: " << git_error_last()->message << std::endl;
        git_index_free(index);
        git_repository_free(repo);
        return;
    }

    size_t changes = git_status_list_entrycount(status);

    if (changes == 0) {
        std::cout << "No changes detected. Nothing to commit." << std::endl;
        // Liberar todo y salir
        git_status_list_free(status);
        git_index_free(index);
        git_repository_free(repo);
        return;
    }

    // Hay cambios → agregar todo al índice
    error = git_index_add_all(index, nullptr, 0, nullptr, nullptr);
    if (error != 0) {
        std::cerr << "Failed to add changes to index: " << git_error_last()->message << std::endl;
        git_status_list_free(status);
        git_index_free(index);
        git_repository_free(repo);
        return;
    }

    // Escribir cambios en el índice
    git_index_write(index);
    git_index_write_tree(&treeOid, index);

    // Crear árbol
    error = git_tree_lookup(&tree, repo, &treeOid);
    if (error != 0) {
        std::cerr << "Failed to lookup tree: " << git_error_last()->message << std::endl;
        git_status_list_free(status);
        git_index_free(index);
        git_repository_free(repo);
        return;
    }

    // Crear firma del commit
    if (git_signature_now(&sig, "Default User", "user@example.com") != 0) {
        std::cerr << "Failed to create signature: " << git_error_last()->message << std::endl;
        git_tree_free(tree);
        git_status_list_free(status);
        git_index_free(index);
        git_repository_free(repo);
        return;
    }

    // Crear el mensaje de commit basado en fecha y hora
    std::time_t now = std::time(nullptr);
    char commitMessage[100];
    std::strftime(commitMessage,
                  sizeof(commitMessage),
                  "Commit at %Y-%m-%d %H:%M:%S",
                  std::localtime(&now));

    // Buscar el último commit (HEAD), si existe
    if (git_revparse_single(&parent_commit, repo, "HEAD") == 0) {
        // HEAD existe, commit con padre
        git_commit *parents[] = {(git_commit *) parent_commit};

        error = git_commit_create(
            &commitOid, repo, "HEAD", sig, sig, nullptr, commitMessage, tree, 1, parents);
        git_object_free(parent_commit);
    } else {
        // No existe HEAD (primer commit)
        error = git_commit_create(
            &commitOid, repo, "HEAD", sig, sig, nullptr, commitMessage, tree, 0, nullptr);
    }

    if (error == 0) {
        std::cout << "Commit created successfully with message: " << commitMessage << std::endl;
    } else {
        std::cerr << "Failed to create commit: " << git_error_last()->message << std::endl;
    }

    // Liberar memoria
    git_tree_free(tree);
    git_status_list_free(status);
    git_signature_free(sig);
    git_index_free(index);
    git_repository_free(repo);
}

std::vector<std::string> VersionManager::listVersions()
{
    std::vector<std::string> branches;
    git_repository *repo = nullptr;
    git_branch_iterator *iter = nullptr;
    git_reference *ref = nullptr;
    git_branch_t type;

    // Log de inicio del proceso de listado de versiones
    std::cout << "Attempting to list versions in repository at " << projectPath << std::endl;

    // Abrir el repositorio
    int error = git_repository_open(&repo, projectPath.c_str());
    if (error != 0) {
        std::cerr << "Failed to open repository: " << git_error_last()->message << std::endl;
        return branches;
    }
    std::cout << "Repository opened successfully for listing branches." << std::endl;

    // Crear un iterador de ramas locales
    error = git_branch_iterator_new(&iter, repo, GIT_BRANCH_LOCAL);
    if (error != 0) {
        std::cerr << "Failed to create branch iterator: " << git_error_last()->message << std::endl;
        git_repository_free(repo);
        return branches;
    }
    std::cout << "Branch iterator created successfully." << std::endl;

    // Iterar sobre las ramas
    while (git_branch_next(&ref, &type, iter) != GIT_ITEROVER) {
        const char *branchName = nullptr;
        if (git_branch_name(&branchName, ref) == 0) {
            branches.push_back(branchName);
            std::cout << "Found branch: " << branchName << std::endl; // Log de rama encontrada
        } else {
            std::cerr << "Failed to get branch name: " << git_error_last()->message << std::endl;
        }
        git_reference_free(ref);
    }
    git_branch_iterator_free(iter);
    git_repository_free(repo);

    return branches;
}

std::vector<std::string> VersionManager::listCommits()
{
    std::vector<std::string> commits;
    git_repository *repo = nullptr;

    // Abrir el repositorio
    int error = git_repository_open(&repo, projectPath.c_str());
    if (error != 0) {
        std::cerr << "Failed to open repository: " << git_error_last()->message << std::endl;
        return commits;
    }

    git_revwalk *walker = nullptr;
    error = git_revwalk_new(&walker, repo);
    if (error != 0) {
        std::cerr << "Failed to create revision walker: " << git_error_last()->message << std::endl;
        git_repository_free(repo);
        return commits;
    }

    // Configurar el walker para recorrer el historial desde HEAD
    git_revwalk_push_head(walker);
    git_revwalk_sorting(walker, GIT_SORT_TIME); // Ordenar por fecha

    git_oid oid;
    while (git_revwalk_next(&oid, walker) == 0) {
        git_commit *commit = nullptr;
        if (git_commit_lookup(&commit, repo, &oid) == 0) {
            // Obtener el mensaje del commit
            const char *message = git_commit_message(commit);
            const git_signature *author = git_commit_author(commit);
            std::string commitInfo = std::string("Commit by ") + author->name + " - " + message;
            commits.push_back(commitInfo);
            git_commit_free(commit);
        }
    }

    git_revwalk_free(walker);
    git_repository_free(repo);

    return commits;
}
