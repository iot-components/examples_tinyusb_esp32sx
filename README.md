
## How to Build Examples

1. Install latest master branch ESP-IDF 
2. Install ESP-IDF component manager
   1. `. ./export.sh` to add ESP-IDF environment values
   2. `pip install idf-component-manager --upgrade`
3. `idf.py set-target esp32s2`
4. `idf.py build flash monitor`
5. if successful install, project's required components will be downloaded automatically

