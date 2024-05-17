REM "OTA binary file convert"
@if "%1%" == "scpu_slave" (
    copy /B .\Objects\fw_scpu_tmp.bin\ER_RO+.\Objects\fw_scpu_tmp.bin\ER_SPL_OVERLAY+.\Objects\fw_scpu_tmp.bin\ER_SDRAM_RO_1+.\Objects\fw_scpu_tmp.bin\ER_SDRAM .\Objects\fw_scpu_tmp_1.bin
    rd /s/q .\Objects\fw_scpu_tmp.bin
    ..\..\..\..\utils\ota\gen_ota_binary_for_win.exe -scpu .\Objects\fw_scpu_tmp_1.bin .\Objects\fw_scpu1.bin
    copy .\Objects\fw_scpu1.bin ..\..\..\..\utils\board_gen\flash_bin\
    del .\Objects\fw_scpu_tmp_1.bin
) else (
    copy /B .\Objects\fw_scpu_tmp.bin\ER_RO+.\Objects\fw_scpu_tmp.bin\ER_SPL_OVERLAY+.\Objects\fw_scpu_tmp.bin\ER_SDRAM_RO_1+.\Objects\fw_scpu_tmp.bin\ER_SDRAM .\Objects\fw_scpu_tmp_1.bin
    rd /s/q .\Objects\fw_scpu_tmp.bin
    ..\..\..\..\utils\ota\gen_ota_binary_for_win.exe -scpu .\Objects\fw_scpu_tmp_1.bin .\Objects\fw_scpu.bin
    copy .\Objects\fw_scpu.bin ..\..\..\..\utils\board_gen\flash_bin\
    del .\Objects\fw_scpu_tmp_1.bin
)
cd ../..
set target_bin= 
for %%i in (.) do set target_bin=%%~nxi

cd ../../utils/board_gen
python board_gen.py -t %target_bin%