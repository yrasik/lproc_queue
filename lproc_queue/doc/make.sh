#! /bin/bash


LATEX='/usr/local/texlive/2022/bin/x86_64-linux/lualatex'

TEX_SRC='top'
OUTPUT='lproc_queue.pdf'


if [ "$1" = "help" ]; then
  echo '-----------------------------------------------------'
  echo '-----            Команды скрипта                -----'
  echo '-----------------------------------------------------'
  echo '-  help    - справка                                -'
  echo '-  clean   - очистка                                -'
  echo '-----------------------------------------------------'
  exit 0
elif [ "$1" = "clean" ]; then
  echo '-----------------------------------------------------'
  echo '-----     Выполнение очистки папки проекта      -----'
  echo '-----------------------------------------------------'
  find . -type f -name "*.aux" -exec rm -f {} \;
  find . -type f -name "*.toc" -exec rm -f {} \;
  find . -type f -name "*.log" -exec rm -f {} \;
  find . -type f -name "*.out" -exec rm -f {} \;
  find . -type f -name "*eps-converted-to.pdf" -exec rm -f {} \;
  find . -type f -name "*.tmp" -exec rm -f {} \;
  find . -type f -name "*.exa" -exec rm -f {} \;
  find . -type f -name "*.data" -exec rm -f {} \;
  find . -type f -name "*.pygtex" -exec rm -f {} \;
  find . -type f -name "*.pygstyle" -exec rm -f {} \;
  find . -type f -name "*.synctex.gz" -exec rm -f {} \;
  find . -type d -name "_minted-*" -exec rm -rf {} \;
  find . -type d -name "svg-inkscape" -exec rm -rf {} \;
  exit 0
fi


echo '-----------------------------------------------------'
echo '----- Проверка компонентов для сборки документа -----'
echo '-----------------------------------------------------'
echo 'Предполагается, что система Ubuntu 2018+'

if [ -e "${LATEX}" ]; then
  echo 'lualatex                  +'
else
  echo 'ERROR: Не найден lualatex. Установите TeXLive 2018+'
  exit 1
fi

if [ -e '/usr/bin/pygmentize' ] || [ -e '/usr/local/bin/pygmentize' ]; then
  echo 'pygmentize                +'
else
  echo 'ERROR: Не найден pygmentize.'
  exit 1
fi

if [ -e "/usr/share/fonts/truetype/PT/PT-Astra-Serif_Regular.ttf" ]; then
  echo 'font: PT Astra Serif      +'
else
  echo 'ERROR: Не найден шрифт PT Astra Serif.'
  exit 1
fi

if [ -e "/usr/share/fonts/truetype/PT/PT-Astra-Sans_Regular.ttf" ]; then
  echo 'font: PT Astra Sans       +'
else
  echo 'ERROR: Не найден шрифт PT Astra Sans.'
  exit 1
fi

if [ -e "/usr/share/fonts/truetype/PT/PTM55FT.ttf" ]; then
  echo 'font: PT Mono             +'
else
  echo 'ERROR: Не найден шрифт PT Mono.'
  exit 1
fi





#${LATEX} -synctex=1 -interaction=stopmode --shell-escape -8bit "${TEX_SRC}.tex"
${LATEX} -synctex=1 -interaction=nonstopmode --shell-escape -8bit "${TEX_SRC}.tex"


if [ -f "${TEX_SRC}.pdf" ]; then
  mv "${TEX_SRC}.pdf"  "${OUTPUT}"
else
  echo 'Файл не был создан'
fi