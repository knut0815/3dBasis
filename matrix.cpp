#include "matrix.hpp"

// Fock space inner product between two monomials
coeff_class InnerFock(const Mono& A, const Mono& B) {
	//B.ChangePm(0, B.Pm(0)+1); // this will be reverted in MassMatrixTerm below
	return MatrixInternal::MassMatrixTerm(A, B, true);
	//return MatrixInternal::MassMatrixTerm(A, B, true)/A.NParticles();
}

// creates a gram matrix for the given basis using the Fock space inner product
DMatrix GramFock(const Basis<Mono>& basis) {
	DMatrix gramMatrix(basis.size(), basis.size());
	for (std::size_t i = 0; i < basis.size(); ++i) {
		gramMatrix(i, i) = InnerFock(basis[i], basis[i]);
		for (std::size_t j = i+1; j < basis.size(); ++j) {
			gramMatrix(i, j) = InnerFock(basis[i], basis[j]);
			gramMatrix(j, i) = gramMatrix(i, j);
		}
	}
	return gramMatrix;
}

// creates a mass matrix M for the given monomials. To get the mass matrix of a 
// basis of primary operators, one must express the primaries as a matrix of 
// vectors, A, and multiply A^T M A.
DMatrix MassMatrix(const Basis<Mono>& basis) {
	DMatrix massMatrix(basis.size(), basis.size());
	for (std::size_t i = 0; i < basis.size(); ++i) {
		massMatrix(i, i) = MatrixInternal::MassMatrixTerm(basis[i], basis[i], 
																		false);
		for (std::size_t j = i+1; j < basis.size(); ++j) {
			massMatrix(i, j) = MatrixInternal::MassMatrixTerm(basis[i], basis[j], 
																		false);
			massMatrix(j, i) = massMatrix(i, j);
		}
	}
	return massMatrix;
}

namespace MatrixInternal {

MatrixTerm_Intermediate::MatrixTerm_Intermediate(const size_t n): coefficient(1),
	uPlus(n), uMinus(n), yTilde(n) {
}

void MatrixTerm_Intermediate::Resize(const size_t n) {
	uPlus.resize(n);
	uMinus.resize(n);
	yTilde.resize(n);
}

// this n would be called (n-1) in Zuhair's equations
MatrixTerm_Final::MatrixTerm_Final(const size_t n): coefficient(1), uPlus(n), 
	uMinus(n), sinTheta(n-1), cosTheta(n-1) {
}

// maybe should be rvalue references instead? hopefully it's the same
MatrixTerm_Final::MatrixTerm_Final(const coeff_class coefficient, const std::vector<char>& uPlus, 
		const std::vector<char>& uMinus, const std::vector<char>& sinTheta,
		const std::vector<char>& cosTheta): coefficient(coefficient), 
	uPlus(uPlus), uMinus(uMinus), sinTheta(sinTheta), cosTheta(cosTheta) {
}

// notice that there are fewer Thetas than Us, so the vectors aren't all the
// same size; also, the n here would be called (n-1) in Zuhair's equations
void MatrixTerm_Final::Resize(const size_t n) {
	uPlus.resize(n);
	uMinus.resize(n);
	sinTheta.resize(n-1);
	cosTheta.resize(n-1);
}

coeff_class MassMatrixTerm(const Mono& A, const Mono& B, 
		const bool isInnerProduct) {
	//std::cout << "TERM: " << A.HumanReadable() << " x " << B.HumanReadable() 
		//<< std::endl;
	//B.ChangePm(0, B.Pm(0)-1);

	// degeneracy factors result from turning the ordered monomials into 
	// symmetric polynomials
	coeff_class degeneracy = 1;
	for(auto& count : A.CountIdentical()) degeneracy *= Factorial(count);
	for(auto& count : B.CountIdentical()) degeneracy *= Factorial(count);

	std::vector<std::size_t> permA(A.PermutationVector());
	std::vector<std::size_t> permB(B.PermutationVector());
	std::vector<MatrixTerm_Final> fFromA, fFromB, combinedFs;
	coeff_class total = 0;
	// there's no reason to be using these permutation vectors instead of 
	// permuting xAndy directly
	do {
		fFromA = MatrixTermsFromMono_Permuted(A, permA);
		do {
			fFromB = MatrixTermsFromMono_Permuted(B, permB);
			combinedFs = CombineTwoFs(fFromA, fFromB);
			total += FinalResult(combinedFs, isInnerProduct);
		} while (std::next_permutation(permB.begin(), permB.end()));
	} while (std::next_permutation(permA.begin(), permA.end()));

	// this somewhat dubious adjustment is presumably due to an error in
	// Zuhair's formula (it appears in his code as well)
	if (A.NParticles() >= 3) total /= 2;

	return degeneracy*A.Coeff()*B.Coeff()*total;
}

// takes a Mono and gets the exponents of the u and theta variables times some
// coefficient; the vector returned here contains the components of the sum
// constituting the answer, each with its own exponents and coefficient
std::vector<MatrixTerm_Final> MatrixTermsFromMono(const Mono& input) {
	std::array<std::vector<char>,2> xAndy(ExponentExtractXY(input));
	return MatrixTermsFromXandY(xAndy, input.NParticles());
}

// this is a silly function and we should just be generating xAndy outside
// and permuting it directly instead of calling this over and over
std::vector<MatrixTerm_Final> MatrixTermsFromMono_Permuted(const Mono& input,
		const std::vector<std::size_t>& permutationVector) {
	std::array<std::vector<char>,2> xAndy(ExponentExtractXY(input));
	//std::cout << "Extracted " << xAndy[0] << " from " << input;
	xAndy = PermuteXandY(xAndy, permutationVector);
	//std::cout << " and permuted it to " << xAndy[0] << std::endl;
	return MatrixTermsFromXandY(xAndy, input.NParticles());
}

std::vector<MatrixTerm_Final> MatrixTermsFromXandY(
		const std::array<std::vector<char>,2>& xAndy, const int nParticles) {
	std::vector<char> uFromX(ExponentUFromX(xAndy[0]));
	//std::cout << "uFromX: " << uFromX << std::endl;
	std::vector<MatrixTerm_Intermediate> inter(ExponentYTildeFromY(xAndy[1]));
	std::vector<MatrixTerm_Final> ret(ExponentThetaFromYTilde(inter));
	// u has contributions from both x and y, so we have to combine them
	if (ret.size() == 0) ret.emplace_back(nParticles - 1);
	for (auto& term : ret) {
		if (term.uPlus.size() < uFromX.size()/2) {
			term.uPlus.resize(uFromX.size()/2, 0);
			term.uMinus.resize(uFromX.size()/2, 0);
			term.sinTheta.resize(uFromX.size()/2 - 1, 0);
			term.cosTheta.resize(uFromX.size()/2 - 1, 0);
		}
		for (auto i = 0u; i < term.uPlus.size(); ++i) {
			term.uPlus[i] += uFromX[i];
			term.uMinus[i] += uFromX[term.uPlus.size() + i];
		}
	}
	if (ret.size() == 0) {
		std::cerr << "Warning: returning an empty set of MatrixTermsFromMono. "
			<< "an empty set should contain one with coefficient 1 and all "
			<< "exponents set to 0." << std::endl;
	}
	return ret;
}

// exponent transformations ---------------------------------------------------
//
// rather than transforming the variables themselves, these take in a list of 
// exponents in one coordinate system and give out lists of exponents in the new 
// coordinate system.

// just gets the exponents of each x and y, so we don't have to worry about
// things like dividing by P or \mu
//
// note that I'm storing the Dirichlet-mandated P_- on each particle but Zuhair
// isn't, so we have to subtract that off
std::array<std::vector<char>,2> ExponentExtractXY(const Mono& extractFromThis) {
	std::vector<char> x, y;
	for (auto i = 0u; i < extractFromThis.NParticles(); ++i) {
		x.push_back(extractFromThis.Pm(i) - 1);
		y.push_back(extractFromThis.Pt(i));
	}
	return {{x, y}};
}

// this function is pointless; xAndy should be generated once and permuted
// directly instead of remaking it over and over again
std::array<std::vector<char>,2> PermuteXandY(
		const std::array<std::vector<char>,2>& xAndy,
		const std::vector<std::size_t>& permutationVector) {
	std::array<std::vector<char>,2> output;
	output[0].resize(xAndy[0].size());
	output[1].resize(xAndy[1].size());
	for (std::size_t i = 0; i < permutationVector.size(); ++i) {
		output[0][i] = xAndy[0][permutationVector[i]];
		output[1][i] = xAndy[1][permutationVector[i]];
	}
	//std::cout << "Permuted " << xAndy[0] << " to " << output[0] << " using "
		//<< permutationVector << std::endl;
	return output;
}

// goes from x to u using Zuhair's (4.21); returned vector has a list of all u+ 
// in order followed by a list of all u- in order
std::vector<char> ExponentUFromX(const std::vector<char>& x) {
	if (x.size() < 2) {
		std::cerr << "Error: asked to do exponent transform from X to U but "
			<< "there were only " << x.size() << " entries in X." << std::endl;
		return {};
	}

	std::vector<char> u(2*x.size() - 2);
	// the terms from x_1 through x_{n-1} are regular
	for (auto i = 0u; i < x.size()-1; ++i) {
		u[i] = 2*x[i];
		for (auto j = 0u; j < i; ++j) {
			u[x.size()-1 + j] += 2*x[i];
		}
	}
	// the last term, from x_n, is different
	for (auto j = 0u; j < x.size()-1; ++j) {
		u[x.size()-1 + j] += 2*x.back();
	}

	//std::cout << "Converted " << x << " into " << u << std::endl;
	return u;
}

// convert from y to y-tilde following (4.26).
//
// this is by far the most intensive of the coordinate transformations: it has
// u biproducts in addition to the yTilde that you want, but worst of all it has
// two terms, so you end up with a sum of return terms, each with some 
// binomial-derived coefficient.
std::vector<MatrixTerm_Intermediate> ExponentYTildeFromY(const std::vector<char>& y) {
	std::vector<MatrixTerm_Intermediate> ret;
	// i here is the i'th particle (pair); each one only sees those whose
	// numbers are lower, so we can treat them using lower particle numbers
	//
	// this loop goes to y.size()-1 because the last term, y_n, is special
	for (auto i = 0u; i < y.size()-1; ++i) {
		Multinomial::Initialize(i, y[i]);
		for (char l = 0; l <= y[i]; ++l) {
			for (const auto& nAndm : Multinomial::GetMVectors(i, y[i]-l)) {
				MatrixTerm_Intermediate newTerm(YTildeTerm(i, y[i], l, 
							std::vector<char>(nAndm.begin()+1, nAndm.end())));
				ret.push_back(std::move(newTerm));
			}
		}
	}
	// handle y_n separately here
	Multinomial::Initialize(y.size()-1, y.back());
	for (char l = 0; l < y.back(); ++l) {
		for (const auto& nAndm : Multinomial::GetMVectors(y.size()-1, y.back() - l)) {
			MatrixTerm_Intermediate lastTerm(YTildeLastTerm(y.size() - 1, 
						y.back(), l, 
						std::vector<char>(nAndm.begin()+1, nAndm.end()) ) );
			ret.push_back(std::move(lastTerm));
		}
	}
	return ret;
}

// part of the transformation in coordinates.pdf; mVectors must have length i,
// which accounts for the 0-indexing in C++ (it has length i-1 in the PDF).
MatrixTerm_Intermediate YTildeTerm(const unsigned int i, const char a, 
		const char l, const std::vector<char>& mVector) {
	MatrixTerm_Intermediate ret(i+1);

	for (auto j = 0u; j <= i-1; ++j) {
		ret.uPlus[j] = mVector[j];
		ret.yTilde[j] = mVector[j];
		ret.uMinus[j] = a;
		for (auto k = j+1; k < i; ++k) {
			ret.uMinus[j] += mVector[k];
		}
	}
	ret.uPlus[i] = a;
	ret.uMinus[i] = l;
	ret.yTilde[i] = l;

	ret.coefficient = YTildeCoefficient(a, l, mVector);
	return ret;
}

// this is the same idea as the above, just for the special term with i=n
MatrixTerm_Intermediate YTildeLastTerm(const unsigned int n, const char a, 
		const char l, const std::vector<char>& mVector) {
	MatrixTerm_Intermediate ret(n-1);
	ret.uPlus = mVector;
	ret.yTilde = mVector;

	for (auto j = 1u; j <= n-1; ++j) {
		ret.uMinus[j-1] = a;
		for (auto k = j+1; k <= n-1; ++k) {
			ret.uMinus[j-1] += mVector[k-1];
		}
	}
	ret.uPlus[n-2] += l;
	ret.yTilde[n-2] += l;

	ret.coefficient = YTildeLastCoefficient(a, l, mVector);
	return ret;
}

// the coefficient of a YTildeTerm, i.e. everything that's not a u or yTilde
coeff_class YTildeCoefficient(const char a, const char l, 
		const std::vector<char>& mVector) {
	coeff_class ret = ExactBinomial(a, l);
	ret *= Multinomial::Choose(mVector.size(), a-l, mVector);
	if (a-l % 2 == 1) ret = -ret;
	return ret;
}

// The coefficient of a YTildeLastTerm, i.e. everything that's not a u or yTilde
// Basically the same as the YTildeCoefficient; only the sign is different
coeff_class YTildeLastCoefficient(const char a, const char l, 
		const std::vector<char>& mVector) {
	coeff_class ret = ExactBinomial(a, l);
	ret *= Multinomial::Choose(mVector.size(), a-l, mVector);
	if (a % 2 == 1) ret = -ret;
	return ret;
}

// convert from y-tilde to sines and cosines of theta following (4.32).
//
// returned vector has sines of all components in order followed by all cosines
std::vector<MatrixTerm_Final> ExponentThetaFromYTilde(
		std::vector<MatrixTerm_Intermediate>& intermediateTerms) {
	std::vector<MatrixTerm_Final> ret;

	for (auto& term : intermediateTerms) {
		// sine[i] appears in all yTilde[j] with j > i (strictly greater)
		std::vector<char> sines(term.yTilde.size()-1, 0);
		for (auto i = 0u; i < sines.size(); ++i) {
			for (auto j = i+1; j < term.yTilde.size(); ++j) {
				sines[i] += term.yTilde[j];
			}
		}
		ret.emplace_back(term.coefficient,
				std::move(term.uPlus), 
				std::move(term.uMinus),
				std::move(sines),
				std::move(term.yTilde) ); // this brings one spurious component
	}

	return ret;
}

std::vector<char> AddVectors(const std::vector<char>& A, 
		const std::vector<char>& B) {
	std::vector<char> output(std::max(A.size(), B.size()), 0);
	for (auto i = 0u; i < std::min(A.size(), B.size()); ++i) {
		output[i] = A[i] + B[i];
	}
	if (A.size() > B.size()) {
		for (auto i = B.size(); i < A.size(); ++i) {
			output[i] = A[i];
		}
	}
	if (B.size() > A.size()) {
		for (auto i = A.size(); i < B.size(); ++i) {
			output[i] = B[i];
		}
	}
	//std::cout << A << " + " << B << " = " << output << std::endl;
	return output;
	/* Better version:
	 * const std::vector<char>* aP = &A;
	 * const std::vector<char>* bP = &B;
	 * if (A.size() < B.size()) std::swap(aP, bP);
	 * std::vector<char> output(*aP);
	 * for (std::size_t i = 0; i < bP->size(); ++i) output[i] += (*bP)[i];
	 * return output;
	 */
}

/*std::vector<char> AddVectors(const std::vector<char>& A, 
		std::vector<char> B, const std::vector<std::size_t>& permutation) {
	std::vector<char> output(std::max(A.size(), B.size()), 0);
	// if this is slow, we should pad both A and B to full size before calling
	if (B.size() < permutation.size()) B.resize(permutation.size(), 0);

	for (auto i = 0u; i < std::min(A.size(), B.size()); ++i) {
		output[i] = A[i] + B[permutation[i]];
	}
	if (A.size() > B.size()) {
		for (auto i = B.size(); i < A.size(); ++i) {
			output[i] = A[i];
		}
	}
	if (B.size() > A.size()) {
		for (auto i = A.size(); i < B.size(); ++i) {
			output[i] = B[permutation[i]];
		}
	}
	std::cout << A << " + " << B << " = " << output << std::endl;
	return output;
}*/

// combines two u-and-theta coordinate wavefunctions (called F in Zuhair's
// notes), each corresponding to one monomial. Each is a sum over many terms, so
// combining them involves multiplying out two sums.
std::vector<MatrixTerm_Final> CombineTwoFs(const std::vector<MatrixTerm_Final>& F1,
		const std::vector<MatrixTerm_Final>& F2) {
	/*std::cout << "Combining two Fs with sizes " << F1.size() << " and " 
		<< F2.size() << std::endl;*/
	std::vector<MatrixTerm_Final> ret;
	for (auto& term1 : F1) {
		for (auto& term2 : F2) {
			ret.emplace_back(term1.coefficient * term2.coefficient,
					AddVectors(term1.uPlus, term2.uPlus),
					AddVectors(term1.uMinus, term2.uMinus),
					AddVectors(term1.sinTheta, term2.sinTheta),
					AddVectors(term1.cosTheta, term2.cosTheta));
		}
	}
	//std::cout << "Combined Fs has " << ret.size() << " terms." << std::endl;
	return ret;
}

coeff_class FinalResult(const std::vector<MatrixTerm_Final>& exponents,
		const bool isInnerProduct) {
	if (exponents.size() == 0) {
		std::cout << "No exponents detected; returning prefactor." << std::endl;
		if (isInnerProduct) {
			return InnerProductPrefactor(2);
		} else {
			return MassMatrixPrefactor(2);
		}
	}
	auto n = exponents.front().uPlus.size() + 1;
	coeff_class totalFromIntegrals = 0;
	for (const auto& term : exponents) {
		//std::cout << "TERM: " << term.coefficient << ", " << term.uPlus
			//<< ", " << term.uMinus << std::endl;
		coeff_class integral = term.coefficient;

		// do the u integrals first; the first one is special because its
		// uPlus power is reduced by 2 unless this is an inner product
		if (isInnerProduct) {
			integral *= UIntegral(term.uPlus[0] + 3, 5*n - 7 + term.uMinus[0]);
		} else {
			integral *= UIntegral(term.uPlus[0] + 1, 5*n - 7 + term.uMinus[0]);
		}
		for (auto i = 1u; i < n-1; ++i) {
			//std::cout << "u+ = " << (int)term.uPlus[i] << "; u- = "
				//<< (int)term.uMinus[i] << std::endl;
			integral *= UIntegral(term.uPlus[i] + 3, 
					5*(n - i) - 7 + term.uMinus[i]);
		}

		// now the theta integrals; sineTheta.size() is n-2, but cosTheta.size()
		// is n-1 with the last component being meaningless. All but the last 
		// one are short, while the last one is long
		//
		// these have constant terms which differ from Zuhair's because his i
		// starts at 1 instead of 0
		if (n >= 3) {
			for (auto i = 0u; i < n-3; ++i) {
				integral *= ThetaIntegral_Short(n - 3 - i + term.sinTheta[i],
						term.cosTheta[i] );
			}
			integral *= ThetaIntegral_Long(term.sinTheta[n-3], term.cosTheta[n-3]);
		}

		//std::cout << "Adding this integral on the pile: " << integral 
			//<< std::endl;
		totalFromIntegrals += integral;
	}
	/*std::cout << "Returning FinalResult = " << InnerProductPrefactor(n) << " * " 
		<< totalFromIntegrals << std::endl;
	return InnerProductPrefactor(n) * totalFromIntegrals;*/
	//std::cout << "Returning FinalResult = " << MassMatrixPrefactor(n) << " * " 
		//<< totalFromIntegrals << std::endl;
	if (isInnerProduct) {
		return InnerProductPrefactor(n) * totalFromIntegrals;
	} else {
		return MassMatrixPrefactor(n) * totalFromIntegrals;
	}
}

coeff_class InnerProductPrefactor(const char n) {
	return MassMatrixPrefactor(n)/n;
}

coeff_class MassMatrixPrefactor(const char n) {
	coeff_class denominator = std::tgamma(n); // tgamma is the "true" gamma fcn
	denominator *= std::pow(16, n-1);
	denominator *= std::pow(M_PI, 2*n-3);
	//std::cout << "PREFACTOR: " << 2/denominator << std::endl;
	return 2/denominator;
}

// integrals ------------------------------------------------------------------

// cache results using hash tables keyed by a and b
namespace {
	std::unordered_map<std::array<coeff_class,2>, coeff_class,
		boost::hash<std::array<coeff_class,2>> > uCache;
	std::unordered_map<std::array<coeff_class,2>, coeff_class,
		boost::hash<std::array<coeff_class,2>> > thetaCache;
} // anonymous namespace

// this is the integral over the "u" variables; it uses a hypergeometric 
// identity to turn 2F1(a, b, c, -1) -> 2^(-a)*2F1(a, c-b, c, 1/2)
//
// follows the conventions of Zuhair's 5.34; a is the exponent of u_i+ and
// b is the exponent of u_i-
coeff_class UIntegral(const coeff_class a, const coeff_class b) {
	//std::cout << "UIntegral(" << a << ", " << b << ")" << std::endl;
	std::array<coeff_class,2> abArray{{a,b}};
	if (b < a) std::swap(abArray[0], abArray[1]);
	if (uCache.count(abArray) == 1) return uCache.at(abArray);

	coeff_class ret = gsl_sf_hyperg_2F1(1, (a+b)/2 + 2, b/2 + 2, 0.5)/(b + 2);
	ret += gsl_sf_hyperg_2F1(1, (a+b)/2 + 2, a/2 + 2, 0.5)/(a + 2);
	ret *= std::pow(std::sqrt(2), -(a+b));
	uCache.emplace(abArray, ret);
	return ret;
}

// this is the integral over the "theta" veriables from 0 to pi; it implements 
// Zuhair's 5.35, where a is the exponent of sin(theta) and b is the exponent of 
// cos(theta).
//
// results are cached by (a,b); since a and b are symmetric, we only store the
// results with a <= b, swapping the two parameters if they're the other order
coeff_class ThetaIntegral_Short(const coeff_class a, const coeff_class b) {
	if (static_cast<int>(b) % 2 == 1) return 0;
	std::array<coeff_class,2> abArray{{a,b}};
	if (b < a) std::swap(abArray[0], abArray[1]);
	if (thetaCache.count(abArray) == 1) return thetaCache.at(abArray);

	coeff_class ret = std::exp(std::lgamma((1+a)/2) + std::lgamma((1+b)/2) 
			- std::lgamma((2 + a + b)/2) );
	thetaCache.emplace(abArray, ret);
	return ret;
}

// this is the integral over the "theta" veriables from 0 to 2pi; it implements 
// Zuhair's 5.36, where a is the exponent of sin(theta) and b is the exponent of
// cos(theta).
coeff_class ThetaIntegral_Long(const coeff_class a, const coeff_class b) {
	if (static_cast<int>(a) % 2 == 1) return 0;
	return 2*ThetaIntegral_Short(a, b);
}

} // namespace MatrixInternal
