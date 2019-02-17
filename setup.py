from setuptools import setup, Extension

setup(
    name = 'bloomgms',
    version = '1.0.1',
    description = 'Genome mappability estimation using Bloom Filter',
    url = "https://github.com/latur/BloomGMS",
    author = 'Igor V.',
    author_email = 'latur@me.com',
    ext_modules = [Extension(
      'bloomgms', 
      ['src/bloomgms.c'], 
      include_dirs = ['src'],
      extra_compile_args=['-std=c99', '-m64', '-O3']
    )],
    py_modules = ['bloomgms']
)

# rm -rf bloomgms* dist build
# python3 setup.py build
# python3 setup.py install
