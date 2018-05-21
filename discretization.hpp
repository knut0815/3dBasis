#ifndef DISCRETIZATION_HPP
#define DISCRETIZATION_HPP

#include <array>
#include <vector>
#include <cmath>
#include <unordered_map>
#include <iostream>

#include <boost/functional/hash.hpp>
#include <gsl/gsl_sf_gamma.h> // for beta function

#include "constants.hpp"
#include "hypergeo.hpp"

SMatrix DiscretizePolys(const DMatrix& polysOnMinBasis, 
                        std::size_t partitions);

// direct matrices ------------------------------------------------------------

DMatrix MuPart(const std::size_t partitions, const MATRIX_TYPE type);
DMatrix MuPart_Kinetic(const std::size_t partitions);

// same-n interactions --------------------------------------------------------

const DMatrix& MuPart_NtoN(const unsigned int n, 
                           std::array<char,2> exponents, 
                           const std::size_t partitions);
const DMatrix& MuPart_2to2(const std::size_t partitions);

coeff_class NtoNWindow_Less(const std::array<char,2>& exponents,
                       const std::array<builtin_class,2>& mu1sq_ab,
                       const std::array<builtin_class,2>& mu2sq_ab);
coeff_class NtoNWindow_Less_Special(const builtin_class r, 
                                const std::array<builtin_class,2>& mu1sq_ab, 
                                const std::array<builtin_class,2>& mu2sq_ab);
coeff_class NtoNWindow_Greater(const std::array<char,2>& exponents,
                       const std::array<builtin_class,2>& mu1sq_ab,
                       const std::array<builtin_class,2>& mu2sq_ab);
coeff_class NtoNWindow_Equal(const std::array<char,2>& exponents,
                             const std::array<builtin_class,2>& musq_ab);
coeff_class NtoNWindow_Equal_Term(const std::array<builtin_class,2>& musq_ab,
                                  const builtin_class arg, 
                                  const builtin_class r,
                                  const bool useMuB);
builtin_class NtoNWindow_Equal_Hypergeometric(const builtin_class arg, 
                                              const builtin_class r,
                                              const builtin_class x);

// n+2 interactions -----------------------------------------------------------

const DMatrix& MuPart_NPlus2(const std::array<char,2>& nr, 
                             const std::size_t partitions);

coeff_class NPlus2Window(const char n, const char r,
        const std::array<builtin_class,2>& mu1_ab,
        const std::array<builtin_class,2>& mu2_ab);
coeff_class NPlus2Window_Equal(const char n, const char r, 
        const builtin_class mu_a, const builtin_class mu_b);

// memoized interfaces to hypergeometric functions ----------------------------
coeff_class Hypergeometric2F1(const builtin_class a, const builtin_class b,
                              const builtin_class c, const builtin_class x);
coeff_class Hypergeometric3F2(const builtin_class a1, const builtin_class a2,
                              const builtin_class a3, const builtin_class b1,
                              const builtin_class b2, const builtin_class x);
coeff_class Hypergeometric3F2(const std::array<builtin_class,3>& a, 
                              const std::array<builtin_class,2>& b, 
                              const builtin_class x);
coeff_class Hypergeometric3F2(const std::array<builtin_class,6>& params);
coeff_class Hypergeometric3F2_Reg(const builtin_class a1, 
                                  const builtin_class a2,
                                  const builtin_class a3,
                                  const builtin_class b1,
                                  const builtin_class b2,
                                  const builtin_class x);
coeff_class Hypergeometric3F2_Reg(const std::array<builtin_class,3>& a, 
        const std::array<builtin_class,2>& b, const builtin_class x);
coeff_class Hypergeometric3F2_Reg(const std::array<builtin_class,6>& params);
coeff_class Hypergeometric4F3(const builtin_class a1, const builtin_class a2,
                              const builtin_class a3, const builtin_class a4,
                              const builtin_class b1, const builtin_class b2,
                              const builtin_class b3, const builtin_class x);
coeff_class Hypergeometric4F3(const std::array<builtin_class,4>& a, 
                              const std::array<builtin_class,3>& b, 
                              const builtin_class x);
coeff_class Hypergeometric4F3(const std::array<builtin_class,8>& params);
coeff_class Hypergeometric4F3_Reg(const builtin_class a1, const builtin_class a2,
                                  const builtin_class a3, const builtin_class a4,
                                  const builtin_class b1, const builtin_class b2,
                                  const builtin_class b3, const builtin_class x);
coeff_class Hypergeometric4F3_Reg(const std::array<builtin_class,4>& a, 
                                  const std::array<builtin_class,3>& b, 
                                  const builtin_class x);
coeff_class Hypergeometric4F3_Reg(const std::array<builtin_class,8>& params);

#endif
