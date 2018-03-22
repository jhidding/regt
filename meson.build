# meson build file

project(
    'Regular Triangaluation Adhesion Code', 'cpp',
    version : '0.0.1',
    license : 'Apache v2',
    default_options : ['cpp_std=c++17']
)

subdir('src')

fftw_dep = dependency('fftw3')
fftwf_dep = dependency('fftw3f')
gsl_dep = dependency('gsl')

cgal_dep = declare_dependency(
        compile_args : ['-frounding-math'],
        link_args: ['-lm', '-lCGAL', '-lgmp', '-lboost_thread', '-lmpfr'])

h5_cflags = run_command(['scripts/h5-config.sh', '--cflags']).stdout().strip().split()
h5_libs = run_command(['scripts/h5-config.sh', '--libs']).stdout().strip().split()

local_include = include_directories('./include', './src')

# executable('regt',
#        src_regt_files, src_ply_files, src_base_files, src_ic_files,
#        src_glass_files,
#        dependencies : [fftw_dep, cgal_dep, gsl_dep],
#        cpp_args : ['-fopenmp', '-frounding-math'],
#        link_args : ['-fopenmp'])

# === [ TESTS ] ===

subdir('test')

gtest_dep = dependency('gtest', main : true, required : false)
e = executable('test-test', test_test_files, test_ply_files, src_support_files,
        dependencies : gtest_dep,
        include_directories: local_include)
test('gtest test', e)
