#include "versionmanager.h"
#include <ctime>
#include <git2.h>
#include <iostream>

VersionManager::VersionManager(const std::string &projectPath)
    : projectPath(projectPath)
{
    git_libgit2_init(); // Inicializar libgit2
}

void VersionManager::initializeRepository()
{
    git_repository *repo = nullptr;
    int error = git_repository_init(&repo, projectPath.c_str(), false);

    if (error == 0) {
        std::cout << "Git repository initialized successfully in: " << projectPath << std::endl;
        git_repository_free(repo);
    } else {
        std::cerr << "Failed to initialize Git repository: " << giterr_last()->message << std::endl;
    }
}

void VersionManager::createVersion(const std::string &versionName)
{
    git_repository *repo = nullptr;
    git_object *head = nullptr;

    // Abrir el repositorio
    int error = git_repository_open(&repo, projectPath.c_str());
    if (error != 0) {
        std::cerr << "Failed to open repository: " << giterr_last()->message << std::endl;
        return;
    }

    // Obtener el último commit (HEAD)
    error = git_revparse_single(&head, repo, "HEAD");
    if (error != 0) {
        std::cerr << "Failed to find HEAD: " << giterr_last()->message << std::endl;
        git_repository_free(repo);
        return;
    }

    // Crear la rama
    error = git_branch_create(nullptr, repo, versionName.c_str(), (git_commit *) head, 0);
    if (error == 0) {
        std::cout << "Branch '" << versionName << "' created successfully." << std::endl;
    } else {
        std::cerr << "Failed to create branch: " << giterr_last()->message << std::endl;
    }

    git_object_free(head);
    git_repository_free(repo);
}

void VersionManager::deleteVersion(const std::string &versionName)
{
    git_repository *repo = nullptr;
    git_reference *branchRef = nullptr;

    // Abrir el repositorio
    int error = git_repository_open(&repo, projectPath.c_str());
    if (error != 0) {
        std::cerr << "Failed to open repository: " << giterr_last()->message << std::endl;
        return;
    }

    // Buscar la rama por su nombre
    error = git_branch_lookup(&branchRef, repo, versionName.c_str(), GIT_BRANCH_LOCAL);
    if (error != 0) {
        std::cerr << "Failed to find branch: " << giterr_last()->message << std::endl;
        git_repository_free(repo);
        return;
    }

    // Borrar la rama
    error = git_branch_delete(branchRef);
    if (error == 0) {
        std::cout << "Branch '" << versionName << "' deleted successfully." << std::endl;
    } else {
        std::cerr << "Failed to delete branch: " << giterr_last()->message << std::endl;
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
        std::cerr << "Failed to open repository: " << giterr_last()->message << std::endl;
        return;
    }

    // Buscar la rama por su nombre
    error = git_revparse_single(&target, repo, ("refs/heads/" + versionName).c_str());
    if (error != 0) {
        std::cerr << "Failed to find branch: " << giterr_last()->message << std::endl;
        git_repository_free(repo);
        return;
    }

    // Hacer checkout de la rama
    error = git_checkout_tree(repo, target, nullptr);
    if (error == 0) {
        git_repository_set_head(repo, ("refs/heads/" + versionName).c_str());
        std::cout << "Switched to branch '" << versionName << "' successfully." << std::endl;
    } else {
        std::cerr << "Failed to switch to branch: " << giterr_last()->message << std::endl;
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

    // Abrir el repositorio
    int error = git_repository_open(&repo, projectPath.c_str());
    if (error != 0) {
        std::cerr << "Failed to open repository: " << giterr_last()->message << std::endl;
        return;
    }

    // Obtener el índice (staging area)
    error = git_repository_index(&index, repo);
    if (error != 0) {
        std::cerr << "Failed to get repository index: " << giterr_last()->message << std::endl;
        git_repository_free(repo);
        return;
    }

    // Agregar todos los cambios al índice
    error = git_index_add_all(index, nullptr, 0, nullptr, nullptr);
    if (error != 0) {
        std::cerr << "Failed to add changes to index: " << giterr_last()->message << std::endl;
        git_index_free(index);
        git_repository_free(repo);
        return;
    }

    // Escribir el índice
    git_index_write(index);
    git_index_write_tree(&treeOid, index);

    // Crear el árbol de trabajo
    error = git_tree_lookup(&tree, repo, &treeOid);
    if (error != 0) {
        std::cerr << "Failed to lookup tree: " << giterr_last()->message << std::endl;
        git_index_free(index);
        git_repository_free(repo);
        return;
    }

    // Obtener la fecha actual para usarla como mensaje del commit
    std::time_t now = std::time(nullptr);
    char commitMessage[100];
    std::strftime(commitMessage,
                  sizeof(commitMessage),
                  "Commit at %Y-%m-%d %H:%M:%S",
                  std::localtime(&now));

    // Crear la firma del commit
    git_signature_now(&sig, "Tu Nombre", "tuemail@ejemplo.com");

    // Crear el commit
    error = git_commit_create_v(
        &commitOid, repo, "HEAD", sig, sig, nullptr, commitMessage, tree, 0, nullptr);
    if (error == 0) {
        std::cout << "Commit created successfully with message: " << commitMessage << std::endl;
    } else {
        std::cerr << "Failed to create commit: " << giterr_last()->message << std::endl;
    }

    git_index_free(index);
    git_tree_free(tree);
    git_signature_free(sig);
    git_repository_free(repo);
}

std::vector<std::string> VersionManager::listVersions()
{
    std::vector<std::string> branches;
    git_repository *repo = nullptr;
    git_branch_iterator *iter = nullptr;
    git_reference *ref = nullptr;
    git_branch_t type;

    // Abrir el repositorio
    int error = git_repository_open(&repo, projectPath.c_str());
    if (error != 0) {
        std::cerr << "Failed to open repository: " << giterr_last()->message << std::endl;
        return branches;
    }

    // Iterar sobre las ramas locales
    error = git_branch_iterator_new(&iter, repo, GIT_BRANCH_LOCAL);
    if (error != 0) {
        std::cerr << "Failed to create branch iterator: " << giterr_last()->message << std::endl;
        git_repository_free(repo);
        return branches;
    }

    while (git_branch_next(&ref, &type, iter) != GIT_ITEROVER) {
        const char *branchName = nullptr;
        if (git_branch_name(&branchName, ref) == 0) {
            branches.push_back(branchName);
        }
        git_reference_free(ref);
    }

    git_branch_iterator_free(iter);
    git_repository_free(repo);

    return branches;
}
