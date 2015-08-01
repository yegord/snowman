/* The file is part of Snowman decompiler. */
/* See doc/licenses.asciidoc for the licensing information. */

#pragma once

#include <nc/config.h>

#include <vector>

#include <QCoreApplication>

#include <boost/unordered_map.hpp>

#include <nc/common/CancellationToken.h>
#include <nc/common/LogToken.h>

#include <nc/core/ir/MemoryLocation.h>

#include "CalleeId.h"

namespace nc {

namespace core {
namespace ir {

class Call;
class Functions;
class Jump;
class Term;

namespace dflow {
    class Dataflows;
    class Uses;
}

namespace liveness {
    class Livenesses;
}

namespace calling {

class FunctionSignature;
class Hooks;
class Signatures;
class ReturnHook;

/**
 * This class reconstructs signatures of functions.
 */
class SignatureAnalyzer {
    Q_DECLARE_TR_FUNCTIONS(SignatureAnalyzer)

    Signatures &signatures_;
    const dflow::Dataflows &dataflows_;
    const Hooks &hooks_;
    const liveness::Livenesses &livenesses_;
    const CancellationToken &canceled_;
    const LogToken &log_;

    struct Referrers {
        std::vector<const Function *> functions;
        std::vector<const Call *> calls;
        std::vector<const Jump *> returns;
    };

    /** Mapping from a callee id to the functions, calls, and returns with this id. */
    boost::unordered_map<CalleeId, Referrers> id2referrers_;

    /** Mapping from a function to the list of calls in it.*/
    boost::unordered_map<const Function *, std::vector<const Call *>> function2calls_;

    /** Mapping from a function to the list of returns in it.*/
    boost::unordered_map<const Function *, std::vector<const Jump *>> function2returns_;

    /** Mapping of terms that represent potential return values in the hooks to callee ids. */
    boost::unordered_map<const Term *, CalleeId> speculativeReturnValueTerm2calleeId_;

    /** Mapping from a function to the term use information for this function. */
    boost::unordered_map<const Function *, std::unique_ptr<dflow::Uses>> function2uses_;

    /** Mapping from a callee id to the list of its formal arguments. */
    boost::unordered_map<CalleeId, std::vector<MemoryLocation>> id2arguments_;

    /** Mapping from a call to the list of factual arguments not included into the list of
     *  formal arguments of the respective callee id. */
    boost::unordered_map<const Call *, std::vector<MemoryLocation>> call2extraArguments_;

    /** Mapping from a callee id to the estimated return value location. */
    boost::unordered_map<CalleeId, MemoryLocation> id2returnValue_;

public:
    /**
     * Constructor.
     *
     * \param signatures An object where to store reconstructed signatures.
     * \param dataflows Dataflows.
     * \param hooks Hooks manager.
     * \param livenesses Livenesses.
     * \param canceled Cancellation token.
     * \param log Log token.
     */
    SignatureAnalyzer(Signatures &signatures, const dflow::Dataflows &dataflows, const Hooks &hooks,
                      const liveness::Livenesses &livenesses, const CancellationToken &canceled, const LogToken &log);

    /**
     * Destructor.
     */
    ~SignatureAnalyzer();

    void analyze();

private:
    /**
     * Precomputes various useful mappings.
     */
    void computeMappings();

    /**
     * Precomputes uses information.
     */
    void computeUses();

    /**
     * Computes locations of arguments for all functions.
     */
    void computeArgumentsAndReturnValues();

    /**
     * Recomputes arguments of the function with the given callee id
     * by looking at the function's body and calls to it.
     *
     * \param[in] calleeId Valid callee id.
     *
     * \return True if the arguments have changed, false otherwise.
     */
    bool computeArguments(const CalleeId &calleeId);

    /**
     * Recomputes the return value of the function with the given callee id
     * by looking at the call and return sites.
     *
     * \param[in] calleeId Valid callee id.
     *
     * \return True if the return value has changed, false otherwise.
     */
    bool computeReturnValue(const CalleeId &calleeId);

    /**
     * \param[in] function Valid pointer to a function.
     *
     * \return Memory locations in the function that are read in the function,
     *         but whose value is not defined before it is read.
     */
    std::vector<MemoryLocation> getUndefinedUses(const Function *function);

    /**
     * \param[in] call Valid pointer to a call statement.
     *
     * \return Memory locations that are defined before the call, but never used.
     *         Stack offsets are fixed up in accordance to the reaching stack pointer value.
     */
    std::vector<MemoryLocation> getUnusedDefines(const Call *call);

    /**
     * \param[in] call Valid pointer to a call statement.
     *
     * \return List of memory locations within the locations where the
     *         return value can be passed, which are read after the call.
     */
    std::vector<MemoryLocation> getUsedReturnValueLocations(const Call *call);

    /**
     * \param[in] jump Valid pointer to a return jump.
     *
     * \return List of memory locations within the locations where the
     *         return value can be passed, which are written before the
     *         return and never read.
     */
    std::vector<MemoryLocation> getUnusedReturnValueLocations(const Jump *jump);

    /**
     * \param term Valid pointer to a term.
     * \param memoryLocation Valid memory location.
     *
     * \return memoryLocation, if term is not a speculative return term.
     *         If it is, then the intersection of the currently assumed return
     *         value location and memoryLocation is returned.
     */
    MemoryLocation intersect(const Term *term, const MemoryLocation &memoryLocation);

    /**
     * Computes and sets signatures for all callee ids.
     */
    void computeSignatures();

    /**
     * Reconstructs the signatures of the functions and calls
     * with the given callee id.
     *
     * \param calleeId Valid callee id.
     */
    void computeSignatures(const CalleeId &calleeId);
};

} // namespace calling
} // namespace ir
} // namespace core
} // namespace nc

/* vim:set et sts=4 sw=4: */
