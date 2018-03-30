#ifndef DISCRETIZATION_HPP
#define DISCRETIZATION_HPP

#include <vector>
#include <cmath>
#include <unordered_map>

#include <boost/functional/hash.hpp>
#include <gsl/gsl_sf_hyperg.h>
#include <gsl/gsl_errno.h>

#include "constants.hpp"
#include "mono.hpp"
#include "basis.hpp"

// takes a vector of polynomial states and expands each one into many states 
// with \mu integrated over a small window

// steps:
// 1. determine spectral density \rho(\mu), which is the square inner product of
// O with all \mu_i for which \mu_i^2 == \mu^2. It should be a polynomial in \mu
// normalized so that it gives 1 when integrated over any single \mu window
// 2. each operator given expands into a block of \int d\mu^2 \rho(\mu) f(\mu) O
// where f(\mu) is a function which differs for different Hamiltonian terms. For
// the kinetic term, it's the identity, whereas for the mass term it's \mu^2

class InteractionCache {
    std::size_t partitions;
    coeff_class partWidth;
    std::unordered_map< std::array<char,3>, coeff_class, 
        boost::hash<std::array<char,3>> > cache;

    public:
        void SetPartitions(const std::size_t partitions, 
                const coeff_class partWidth);
        bool HasPartitions(const std::size_t partitions,
                const coeff_class partWidth);

        void Emplace(const std::array<char,3>& key, const coeff_class value);
        bool Contains(const std::array<char,3>& key);
        const coeff_class& operator[](const std::array<char,3>& key) const;
        void Clear();
};

char MuExponent(const Mono& A, const Mono& B);
DVector MuIntegral(const Mono& A, const Mono& B, const std::size_t partitions,
        const coeff_class partitionWidth, const MATRIX_TYPE calculationType);
DVector MuIntegral_Body(const char muExp, const std::size_t partitions,
                const coeff_class partitionWidth);
coeff_class MuNorm(const Mono& A, const std::size_t k, 
        const coeff_class partWidth);
DMatrix PartitionMu_Mass(const Basis<Mono>& minimalBasis, const DMatrix& mass,
		const std::size_t partitions, const coeff_class partWidth);

DMatrix MuPart(const Mono& A, const Mono& B, const std::size_t partitions, 
        const coeff_class partWidth, const MATRIX_TYPE type);
DMatrix MuPart_Kinetic(const std::size_t partitions, 
        const coeff_class partWidth);
DMatrix MuTotal(const Basis<Mono>& minBasis, const std::size_t partitions,
        const coeff_class partWidth, const MATRIX_TYPE calculationType);

coeff_class InteractionMu(const std::array<char,3> r, 
        const std::size_t partitions, const coeff_class partWidth);
coeff_class RIntegral(const char a, const char b, const char c, 
        const coeff_class alpha);
coeff_class Hypergeometric2F1(const coeff_class a, const coeff_class b,
        const coeff_class c, const coeff_class x);

DMatrix DiscretizeMonos(const Basis<Mono>& minBasis, 
        const std::size_t partitions, const coeff_class partWidth);
DMatrix DiscretizePolys(const DMatrix& polysOnMinBasis, 
        const std::size_t partitions);

#endif