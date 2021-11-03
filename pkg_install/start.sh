#!/bin/bash
cd /home/jovyan/pkg/
find /home/jovyan/pkg/ -type f | xargs ls -l
python setup.py bdist_wheel
python -m pip uninstall nb_plot_streamlit
python -m pip install dist/nb_plot_streamlit-0.0.1-py3-none-any.whl --force-reinstall
yes "" | streamlit run launcher/launch.py
