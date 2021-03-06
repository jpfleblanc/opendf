#pragma once 

#include <gftools/fft.hpp>
#include <opendf/config.hpp>

namespace open_df { 
namespace diagrams { 

/// diagram_traits - a set of common definitions for vertex diagrammatics calculations
template <typename LatticeT>
struct diagram_traits { 
    typedef LatticeT lattice_t;
    typedef grid_object<std::complex<double>, bmatsubara_grid, fmatsubara_grid, fmatsubara_grid> vertex_type;
    typedef grid_object<std::complex<double>, fmatsubara_grid, fmatsubara_grid> fvertex_type;
    typedef grid_object<std::complex<double>, fmatsubara_grid> gw_type;

    static constexpr int NDim = lattice_t::NDim; 

    typedef typename gftools::tools::ArgBackGenerator<NDim,kmesh,grid_object,complex_type,fmatsubara_grid>::type gk_type;
    typedef typename gftools::tools::ArgBackGenerator<NDim,kmesh,grid_object,complex_type>::type disp_type;
    typedef typename gftools::tools::ArgBackGenerator<NDim,kmesh,grid_object,complex_type,bmatsubara_grid>::type vertex_eval_type;
    typedef typename gftools::tools::ArgBackGenerator<NDim,kmesh,grid_object,complex_type,bmatsubara_grid, fmatsubara_grid>::type full_diag_vertex_type;

    /// Calculate a static bubble $-T \Sum_k G(i\omega, k) G(i\omega, k+q) (at bosonic freq = 0) for a given GF
    static gk_type calc_static_bubbles(gk_type const& gf);
    /// Calculate a bubble $-T \Sum_k G(i\omega, k) G(i\omega + W, k+q) (at bosonic freq = 0) for a given GF
    static gk_type calc_bubbles(gk_type const& gf, bmatsubara_grid::point W);

    static vertex_eval_type get_max_eigenvalues(gk_type const& bubbles, vertex_type const& bare_vertex, lattice_t const& lattice, bmatsubara_grid::point W);
};


template <typename, bool> class BetheSalpeter;
template <bool Forward> class BetheSalpeter<matrix_type, Forward> { 
public:
    constexpr static bool fwd = Forward;
    matrix_type solve_iterations(size_t n_iter, real_type mix,  bool evaluate_only_order_n);
    matrix_type solve_inversion();

    matrix_type solve(bool eval_iterations, size_t n_iter = 1, real_type mix= 1.0, bool evaluate_only_order_n = false);
    
    BetheSalpeter(matrix_type const& bubble, matrix_type const& vertex, unsigned int verbosity = 0) : bubble_(bubble), vertex_(vertex), verbosity_(verbosity) {}
    std::complex<double> determinant() const { return det_; }
protected:
    matrix_type const& bubble_;
    matrix_type const& vertex_;
    typename matrix_type::Scalar det_ = 1.0;
    unsigned int verbosity_;
};

/// Evaluate Bethe-Salpeter matrix equation
//matrix_type BS(const matrix_type &Chi0, const matrix_type &IrrVertex4, bool forward, bool eval_SC, size_t n_iter, real_type mix,  bool evaluate_only_order_n = false);

// implementation
template <typename LatticeT>
typename diagram_traits<LatticeT>::gk_type diagram_traits<LatticeT>::calc_static_bubbles(gk_type const& GF)
{
    typename diagram_traits<LatticeT>::gk_type out(GF.grids());
    const auto& fgrid = std::get<0>(GF.grids());
    int knorm = GF[0].size();
    for (fmatsubara_grid::point iw1 : fgrid.points())  {
        auto g1 = run_fft(GF[iw1], FFTW_FORWARD);
        out[iw1] = run_fft(g1*g1, FFTW_BACKWARD)/knorm;
        };
    return out / (-fgrid.beta());
}

template <typename LatticeT>
typename diagram_traits<LatticeT>::gk_type diagram_traits<LatticeT>::calc_bubbles(gk_type const& GF, bmatsubara_grid::point W)
{
    if (is_float_equal(W.value(), 0)) return diagram_traits<LatticeT>::calc_static_bubbles(GF); 
    typename diagram_traits<LatticeT>::gk_type GF_shift(GF.grids());
    const auto& fgrid = std::get<0>(GF.grids());
    double beta = fgrid.beta();
    int Wn = BMatsubaraIndex(W.value(), beta);
    for (auto w : fgrid.points()) {  
        if (FMatsubaraIndex(w, beta) + Wn >= fgrid.min_n() && FMatsubaraIndex(w, beta) + Wn < fgrid.max_n())
            GF_shift[w] = GF[w.index() + Wn];
        else GF_shift[w] = 0.0;
        }
    typename diagram_traits<LatticeT>::gk_type out(GF.grids());
    int knorm = GF[0].size();
    for (fmatsubara_grid::point iw1 : fgrid.points())  {
        auto g1 = run_fft(GF[iw1], FFTW_FORWARD);
        auto g2 = run_fft(GF_shift[iw1], FFTW_FORWARD);
        out[iw1] = run_fft(g1*g2, FFTW_BACKWARD)/knorm;
        };
    return out / (-fgrid.beta());
} 

std::complex<double> max_eval(const matrix_type &Chi0, const matrix_type &IrrVertex4); 

template <typename LatticeT>
typename diagram_traits<LatticeT>::vertex_eval_type diagram_traits<LatticeT>::get_max_eigenvalues(gk_type const& bubbles, vertex_type const& vertex, lattice_t const& lattice, bmatsubara_grid::point W)
{
    const matrix_type fvertex = vertex[W].as_matrix(); 
    
}

} // end of namespace diagrams
} // end of namespace open_df
