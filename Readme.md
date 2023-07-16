# EmuClient
## Tested
+ JMS v164, v165, v186, v188, v194
+ TWMS v157
+ MSEA v97, v102

## Settings (.ini)
+ LocalHost.ini
	+ 設定不要
	+ JMS以外の場合はAuthHook=1
+ EmuMain.ini
	+ 設定不要
	+ 特定のバージョンを指定する場合はRegionとVersionを設定する
		+ 例: Region=MSEA, Version=102

## RunEmu
+ 単純なInjectorでEmuLoader.dllをInjectする用途で利用します

## EmuLoader
+ 別のDLLを読み込ませる目的で利用します
+ 実行前に読み込ませるDLLの例
	+ FixThemida.dll
		+ v164など古いバージョンでは実行前に読み込ませないとそもそもクライアントが起動しません
		+ https://github.com/Riremito/FixThemida
	+ LocalHost.dll
		+ 接続先のIPをアドレスを変更するので最初に読み込ませたほうが良いです
		+ https://github.com/Riremito/LocalHost
+ メモリ展開後に読み込ませるDLLの例
	+ EmuMain.dll
		+ MSCRC Bypassを行うため、一番最初に読み込ませるべきです
	+ EmuExtra.dll
		+ その他メモリの書き換えを行うのでMSCRC Bypassより後に読み込ませる必要があります
	+ Packet.dll	
		+ 解析のためにPacketをフックするDLLです、無くてもよいです
		+ https://github.com/Riremito/RirePE