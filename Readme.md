# EmuClient
## RunEmu
+ 単純なInjectorでEmuLoader.dllをInjectする用途で利用します

## EmuLoader
+ 別のDLLを読み込ませる目的で利用します
+ 実行前に読み込ませるDLLの例
	+ FixThemida.dll
		+ 実行前に読み込ませないとそもそもクライアントが起動しません
	+ LocalHost.dll
		+ 接続先のIPをアドレスを変更するので最初に読み込ませたほうが良いです
	+ MultiClient.dll
		+ 多重起動を許可する場合に必要です
+ メモリ展開後に読み込ませるDLLの例
	+ EmuMain.dll
		+ MSCRC Bypassを行うため、一番最初に読み込ませるべきです
	+ EmuExtra.dll
		+ その他メモリの書き換えを行うのでMSCRC Bypassより後に読み込ませる必要があります
	+ Packet.dll	
		+ 解析のためにPacketをフックするDLLです、無くてもよいです