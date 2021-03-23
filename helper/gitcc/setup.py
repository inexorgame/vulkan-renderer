#!/usr/bin/env python

from setuptools import find_packages, setup

setup(
    name="GIT Commit Check",
    version="1.0.0",
    description="Check the commit summary",
    long_description="",
    long_description_content_type='text/x-rst',
    author="Iceflower S",
    author_email="iceflower@gmx.de",
    license='MIT',
    url="",
    classifiers=[
        'Programming Language :: Python :: 3.9',
        'License :: OSI Approved :: MIT License',
        'Development Status :: 5 - Production/Stable',
        'Operating System :: OS Independent',
        'Intended Audience :: Developers',
        'Natural Language :: English',
        'Environment :: Console',
    ],
    keywords='git commit naming checker',
    packages=find_packages(include=['gitcc', 'gitcc.*']),
    python_requires='>=3.9',
    install_requires=[
        "GitPython==3.1.14",
    ],
    zip_safe=True,
    entry_points={
        'console_scripts': [
            'gitcc = gitcc.main:main',
        ],
    },
)
