#include "versionmanager.h"
#include <algorithm>
#include <ctime>
#include <fstream> // Para crear un archivo temporal
#include <git2.h>
#include <iostream>
#include <string>
#include <vector>

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

        // Crear el archivo .gitignore
        std::string gitignorePath = projectPath + "/.gitignore";
        std::ofstream gitignoreFile(gitignorePath);

        // Agregar reglas para ignorar node_modules en backend y frontend
        gitignoreFile << "backend/\n";
        gitignoreFile << "frontend/\n";
        gitignoreFile.close();

        std::cout << ".gitignore file created with node_modules exclusions." << std::endl;
    } else {
        std::cerr << "Failed to initialize Git repository: " << giterr_last()->message << std::endl;
    }

    git_repository_free(repo);
}

void VersionManager::createVersion(const std::string &branchName)
{
    git_repository *repo = nullptr;
    git_reference *newBranchRef = nullptr;
    git_oid commitOid;
    git_index *index = nullptr;
    git_tree *tree = nullptr;
    git_signature *sig = nullptr;

    // Abrir el repositorio
    int error = git_repository_open(&repo, projectPath.c_str());
    if (error != 0) {
        std::cerr << "Failed to open repository: " << giterr_last()->message << std::endl;
        return;
    }

    // Obtener el commit actual en HEAD
    git_object *headCommit = nullptr;
    error = git_revparse_single(&headCommit, repo, "HEAD");
    if (error != 0) {
        std::cerr << "Failed to get HEAD commit: " << giterr_last()->message << std::endl;
        git_repository_free(repo);
        return;
    }

    // Crear la nueva rama a partir del último commit en HEAD
    error = git_branch_create(&newBranchRef, repo, branchName.c_str(), (git_commit *) headCommit, 0);
    if (error != 0) {
        std::cerr << "Failed to create branch: " << giterr_last()->message << std::endl;
        git_object_free(headCommit);
        git_repository_free(repo);
        return;
    }

    std::cout << "Branch '" << branchName << "' created successfully." << std::endl;

    // Cambiar a la nueva rama
    error = git_repository_set_head(repo, ("refs/heads/" + branchName).c_str());
    if (error != 0) {
        std::cerr << "Failed to switch to branch: " << giterr_last()->message << std::endl;
        git_reference_free(newBranchRef);
        git_object_free(headCommit);
        git_repository_free(repo);
        return;
    }

    // Hacer un "checkout" a la nueva rama para asegurarse de que HEAD esté alineado
    git_checkout_options checkoutOpts = GIT_CHECKOUT_OPTIONS_INIT;
    checkoutOpts.checkout_strategy = GIT_CHECKOUT_SAFE;
    error = git_checkout_head(repo, &checkoutOpts);
    if (error != 0) {
        std::cerr << "Failed to checkout branch: " << giterr_last()->message << std::endl;
        git_reference_free(newBranchRef);
        git_object_free(headCommit);
        git_repository_free(repo);
        return;
    }

    // Obtener el índice y agregar todos los cambios para el commit en la nueva rama
    error = git_repository_index(&index, repo);
    if (error != 0) {
        std::cerr << "Failed to get repository index: " << giterr_last()->message << std::endl;
        git_reference_free(newBranchRef);
        git_object_free(headCommit);
        git_repository_free(repo);
        return;
    }

    error = git_index_add_all(index, nullptr, 0, nullptr, nullptr);
    if (error != 0) {
        std::cerr << "Failed to add changes to index: " << giterr_last()->message << std::endl;
        git_index_free(index);
        git_reference_free(newBranchRef);
        git_object_free(headCommit);
        git_repository_free(repo);
        return;
    }

    // Escribir el índice y crear el árbol para el commit en la nueva rama
    git_oid treeOid;
    git_index_write_tree(&treeOid, index);

    error = git_tree_lookup(&tree, repo, &treeOid);
    if (error != 0) {
        std::cerr << "Failed to lookup tree: " << giterr_last()->message << std::endl;
        git_index_free(index);
        git_reference_free(newBranchRef);
        git_object_free(headCommit);
        git_repository_free(repo);
        return;
    }

    // Crear la firma y el mensaje de commit en la nueva rama
    git_signature_now(&sig, "Default User", "user@example.com");
    std::time_t now = std::time(nullptr);
    char commitMessage[100];
    std::strftime(commitMessage,
                  sizeof(commitMessage),
                  "Commit at %Y-%m-%d %H:%M:%S",
                  std::localtime(&now));

    error = git_commit_create_v(
        &commitOid, repo, "HEAD", sig, sig, nullptr, commitMessage, tree, 0, nullptr);
    if (error == 0) {
        std::cout << "Commit created successfully on branch '" << branchName
                  << "' with message: " << commitMessage << std::endl;
    } else {
        std::cerr << "Failed to create commit: " << giterr_last()->message << std::endl;
    }

    // Liberar memoria
    git_index_free(index);
    git_tree_free(tree);
    git_signature_free(sig);
    git_reference_free(newBranchRef);
    git_object_free(headCommit);
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
    git_signature_now(&sig, "Default User", "user@example.com");

    // Crear el commit inicial o de cambios
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

    // Log de inicio del proceso de listado de versiones
    std::cout << "Attempting to list versions in repository at " << projectPath << std::endl;

    // Abrir el repositorio
    int error = git_repository_open(&repo, projectPath.c_str());
    if (error != 0) {
        std::cerr << "Failed to open repository: " << giterr_last()->message << std::endl;
        return branches;
    }
    std::cout << "Repository opened successfully for listing branches." << std::endl;

    // Crear un iterador de ramas locales
    error = git_branch_iterator_new(&iter, repo, GIT_BRANCH_LOCAL);
    if (error != 0) {
        std::cerr << "Failed to create branch iterator: " << giterr_last()->message << std::endl;
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
            std::cerr << "Failed to get branch name: " << giterr_last()->message << std::endl;
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
        std::cerr << "Failed to open repository: " << giterr_last()->message << std::endl;
        return commits;
    }

    git_revwalk *walker = nullptr;
    error = git_revwalk_new(&walker, repo);
    if (error != 0) {
        std::cerr << "Failed to create revision walker: " << giterr_last()->message << std::endl;
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
