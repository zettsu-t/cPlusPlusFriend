FROM streamlit

USER root
RUN mkdir -p /home/jovyan/pkg/config
COPY config /home/jovyan/pkg/config
COPY launcher /home/jovyan/pkg/launcher
COPY patch /home/jovyan/pkg/patch
COPY src /home/jovyan/pkg/src
COPY tests /home/jovyan/pkg/
COPY LICENSE.txt /home/jovyan/pkg/
COPY MANIFEST.in /home/jovyan/pkg/
COPY pyproject.toml /home/jovyan/pkg/
COPY README.md /home/jovyan/pkg/
COPY setup.cfg /home/jovyan/pkg/
COPY setup.py /home/jovyan/pkg/

COPY start.sh /home/jovyan/pkg/start.sh
RUN find /home/jovyan/pkg/ | xargs chown jovyan:users
RUN chmod ugo+x /home/jovyan/pkg/start.sh

USER jovyan
ENTRYPOINT ["/bin/bash", "/home/jovyan/pkg/start.sh"]
