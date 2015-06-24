/* The file is part of Snowman decompiler. */
/* See doc/licenses.asciidoc for the licensing information. */

/* * SmartDec decompiler - SmartDec is a native code to C/C++ decompiler
 * Copyright (C) 2015 Alexander Chernov, Katerina Troshina, Yegor Derevenets,
 * Alexander Fokin, Sergey Levin, Leonid Tsvetkov
 *
 * This file is part of SmartDec decompiler.
 *
 * SmartDec decompiler is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * SmartDec decompiler is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with SmartDec decompiler.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include <nc/config.h>

#include <memory> /* For std::unique_ptr. */

#include <QObject>

#include <nc/common/CancellationToken.h>
#include <nc/common/LogToken.h>

namespace nc {
namespace core {

namespace arch {
    class Instructions;
}

namespace image {
    class Image;
}

namespace ir {
    class Function;
    class Functions;
    class Program;

    namespace calling {
        class Conventions;
        class Hooks;
        class Signatures;
    }
    namespace cflow {
        class Graphs;
    }
    namespace dflow {
        class Dataflows;
    }
    namespace types {
        class Types;
    }
    namespace liveness {
        class Livenesses;
    }
    namespace vars {
        class Variables;
    }
}

namespace likec {
    class Tree;
}

/**
 * This class stores all the information that is required and produced during decompilation.
 */
class Context: public QObject {
    Q_OBJECT

    std::shared_ptr<image::Image> image_; ///< Executable image being decompiled.
    std::shared_ptr<const arch::Instructions> instructions_; ///< Instructions being decompiled.
    std::unique_ptr<ir::Program> program_; ///< Program.
    std::unique_ptr<ir::Functions> functions_; ///< Functions.
    std::unique_ptr<ir::calling::Conventions> conventions_; ///< Assigned calling conventions.
    std::unique_ptr<ir::calling::Hooks> hooks_; ///< Hooks manager.
    std::unique_ptr<ir::calling::Signatures> signatures_; ///< Signatures.
    std::unique_ptr<ir::dflow::Dataflows> dataflows_; ///< Dataflows.
    std::unique_ptr<ir::vars::Variables> variables_; ///< Reconstructed variables.
    std::unique_ptr<ir::cflow::Graphs> graphs_; ///< Structured graphs.
    std::unique_ptr<ir::liveness::Livenesses> livenesses_; ///< Liveness information.
    std::unique_ptr<ir::types::Types> types_; ///< Information about types.
    std::unique_ptr<likec::Tree> tree_; ///< Abstract syntax tree of the LikeC program.
    LogToken logToken_; ///< Log token.
    CancellationToken cancellationToken_; ///< Cancellation token.

public:
    /**
     * Class constructor.
     */
    Context();

    /**
     * Class destructor.
     */
    ~Context();

    /**
     * Sets the executable image being decompiled.
     *
     * \param image Pointer to the image. Can be nullptr.
     */
    void setImage(const std::shared_ptr<image::Image> &image);

    /**
     * \return Pointer to the executable image being decompiled. Can be nullptr.
     */
    std::shared_ptr<image::Image> image() const { return image_; }

    /**
     * Sets the set instructions being decompiled.
     *
     * \param instructions Pointer to the new set of instructions. Can be nullptr.
     */
    void setInstructions(const std::shared_ptr<const arch::Instructions> &instructions);

    /**
     * \returns Pointer to the instructions being decompiled. Can be nullptr.
     */
    const std::shared_ptr<const arch::Instructions> &instructions() const { return instructions_; }

    /**
     * Sets the intermediate representation of the program.
     *
     * \param program Pointer to the program IR. Can be nullptr.
     */
    void setProgram(std::unique_ptr<ir::Program> program);

    /**
     * \return Pointer to the program. Can be nullptr.
     */
    const ir::Program *program() const { return program_.get(); }

    /**
     * Sets the set of functions.
     *
     * \param functions Pointer to the set of functions. Can be nullptr.
     */
    void setFunctions(std::unique_ptr<ir::Functions> functions);

    /**
     * \return Pointer to the set of functions. Can be nullptr.
     */
    ir::Functions *functions() const { return functions_.get(); }

    /**
     * Sets the assigned calling conventions.
     *
     * \param conventions Pointer to the assigned calling conventions. Can be nullptr.
     */
    void setConventions(std::unique_ptr<ir::calling::Conventions> conventions);

    /**
     * \return Pointer to the information on calling conventions of functions. Can be nullptr.
     */
    ir::calling::Conventions *conventions() { return conventions_.get(); }

    /**
     * \return Pointer to the information on calling conventions of functions. Can be nullptr.
     */
    const ir::calling::Conventions *conventions() const { return conventions_.get(); }

    /**
     * Sets the hooks manager.
     *
     * \param hooks Pointer to the hooks manager. Can be nullptr.
     */
    void setHooks(std::unique_ptr<ir::calling::Hooks> hooks);

    /**
     * \return Pointer to the hooks manager. Can be nullptr.
     */
    ir::calling::Hooks *hooks() { return hooks_.get(); }

    /**
     * \return Pointer to the hooks manager. Can be nullptr.
     */
    const ir::calling::Hooks *hooks() const { return hooks_.get(); }

    /**
     * Sets the reconstructed signatures.
     *
     * \param signatures Pointer to the signatures. Can be nullptr.
     */
    void setSignatures(std::unique_ptr<ir::calling::Signatures> signatures);

    /**
     * \return Pointer to the signatures of functions. Can be nullptr.
     */
    ir::calling::Signatures *signatures() { return signatures_.get(); }

    /**
     * \return Pointer to the signatures of functions. Can be nullptr.
     */
    const ir::calling::Signatures *signatures() const { return signatures_.get(); }

    /**
     * Sets the dataflow information for all functions.
     *
     * \param[in] dataflows Pointer to the dataflow information. Can be nullptr.
     */
    void setDataflows(std::unique_ptr<ir::dflow::Dataflows> dataflows);

    /**
     * \return Pointer to the dataflow information for all functions. Can be nullptr.
     */
    ir::dflow::Dataflows *dataflows() { return dataflows_.get(); }

    /**
     * \return Pointer to the dataflow information for all functions. Can be nullptr.
     */
    const ir::dflow::Dataflows *dataflows() const { return dataflows_.get(); }

    /**
     * Sets the information about reconstructed variables.
     *
     * \param[in] variables Pointer to the information about reconstructed variables. Can be nullptr.
     */
    void setVariables(std::unique_ptr<ir::vars::Variables> variables);

    /**
     * \return Pointer to the information about reconstructed variables. Can be nullptr.
     */
    const ir::vars::Variables *variables() const { return variables_.get(); }

    /**
     * Sets the structured graphs for all functions.
     *
     * \param[in] graphs Pointer to the graphs. Can be nullptr.
     */
    void setGraphs(std::unique_ptr<ir::cflow::Graphs> graphs);

    /**
     * \return Pointer to the structured graphs. Can be nullptr.
     */
    ir::cflow::Graphs *graphs() { return graphs_.get(); }

    /**
     * \return Pointer to the structured graphs. Can be nullptr.
     */
    const ir::cflow::Graphs *graphs() const { return graphs_.get(); }

    /**
     * Sets the liveness information for all functions.
     *
     * \param[in] liveness Pointer to the liveness information. Can be nullptr.
     */
    void setLivenesses(std::unique_ptr<ir::liveness::Livenesses> liveness);

    /**
     * \return Pointer to the liveness information for all functions. Can be nullptr.
     */
    ir::liveness::Livenesses *livenesses() { return livenesses_.get(); }

    /**
     * \return Pointer to the liveness information for all functions. Can be nullptr.
     */
    const ir::liveness::Livenesses *livenesses() const { return livenesses_.get(); }

    /**
     * Sets the information about types.
     *
     * \param[in] types Pointer to the information about types. Can be nullptr.
     */
    void setTypes(std::unique_ptr<ir::types::Types> types);

    /**
     * \return Pointer to the information about types. Can be nullptr.
     */
    const ir::types::Types *types() const { return types_.get(); }

    /**
     * Sets the LikeC tree.
     *
     * \param tree Valid pointer to the LikeC tree.
     */
    void setTree(std::unique_ptr<likec::Tree> tree);

    /**
     * \return The LikeC tree. Can be nullptr.
     */
    likec::Tree *tree() const { return tree_.get(); }

    /**
     * Sets cancellation token.
     *
     * \param token Cancellation token.
     */
    void setCancellationToken(const CancellationToken &token) { cancellationToken_ = token; }

    /**
     * \return Cancellation token.
     */
    const CancellationToken &cancellationToken() const { return cancellationToken_; }

    /**
     * Sets the log token.
     *
     * \param token log token.
     */
    void setLogToken(const LogToken &token) { logToken_ = token; }

    /**
     * \return Log token.
     */
    const LogToken &logToken() const { return logToken_; }

    Q_SIGNALS:

    /**
     * Signal emitted when the set of instructions is changed.
     */
    void instructionsChanged();

    /**
     * Signal emitted when LikeC tree is computed.
     */
    void treeChanged();
};

} // namespace core
} // namespace nc

/* vim:set et sts=4 sw=4: */
