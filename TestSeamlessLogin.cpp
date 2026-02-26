// TestSeamlessLogin.cpp : このファイルには 'main' 関数が含まれています。プログラム実行の開始と終了がそこで行われます。
//

#include <Windows.h>
#include <iostream>
#include <fstream>
#include <string>
#include <memory>

using namespace std;

//------------------------------------------------------------
// 送受信モジュール 定義
#import "..\bin\CLCommunication.dll"
using namespace CLCOMMUNICATIONLib;

//------------------------------------------------------------
// 利用者識別番号・パスワード（任意に書き換えてください）
//------------------------------------------------------------
const string USER_ID = "1111111111111";
const string PASSWORD = "Pass0001";

//------------------------------------------------------------
// HTMLテンプレート用置き換え文字：”引継ぎ情報”
const string HTML_REPLACEWORD("<SessionInfo>");


//=========================================================
// TestSeamlessLogin
//
// 概要：e-taxソフト送受信モジュールを用いたシームレスログインの例
// 説明：送受信モジュールで受付システムにログインを行い、その引継ぎ情報を使用して
// 　　　Webブラウザでシームレスログインを実施する例です。
// 　　・利用者識別番号を用いて受付システムにログインします。
// 　　・ログイン完了後にレスポンスデータから引継ぎ情報を取得します。
// 　　・引継ぎ情報をシームレスログインのリクエストパラメタに埋め込み、Webブラウザで開きます。
// 　　・WebブラウザでシームレスログインURLのリクエストを送信され、e-Taxソフト（WEB版）画面がログイン状態で表示されます。
//=========================================================

//------------------------------------------------------------
// 送受信モジュール 初期化処理
boolean initCommModule(CLICommunicationPtr& pComm, const string& iniFileName)
{
	HRESULT hr = S_OK;

	// CLCommunicationインターフェイスの作成
	hr = pComm.CreateInstance(__uuidof(CLCCommunication));
	if (FAILED(hr)) {
		cerr << "CLCommunicationインターフェイスの作成でエラー: " << hr << endl;
		return false;
	}

	// 受付システム接続設定ファイルの設定
	hr = pComm->put_AttestationUrl(_bstr_t(iniFileName.c_str()));
	if (FAILED(hr)) {
		cerr << "受付システムへのLogin認証とメインメニューからのリンク先の設定でエラー: " << hr << endl;
		return false;
	}

	return true;
}

//------------------------------------------------------------
// 利用者識別番号を使用した受付システム認証処理
boolean loginETax(CLICommunicationPtr pComm, const string& uid, const string& password, CLIHttpResponsePtr& pRes)
{
	_variant_t vtRes;
	CLIHttpRequestPtr pReq = NULL;

	//-------------------------------------------
	// 認証処理
	//-------------------------------------------
	// 実行（メインメニュー要求）
	pReq = pComm->CreateRequest("XU00S010");

	// リクエスト送信
	vtRes = pComm->Send();

	//-------------------------------------------
	// 認証実行
	pReq = pComm->CreateRequest("XU00S010_1");

	// リクエストパラメタ：利用者識別番号、暗証番号 の設定
	pReq->SetParameter("sUserId", uid.c_str());
	pReq->SetParameter("sPwd", password.c_str());

	// リクエスト送信
	vtRes = pComm->Send();

	// 正常にリクエストが処理された場合、SU00S020（メインメニュー）に画面遷移する
	if (vtRes != _variant_t("SU00S020")) {
		cerr << "受付システムの画面遷移が失敗しました：" << vtRes.bstrVal << endl;
		return false;
	}

	//-------------------------------------------
	// レスポンスデータ［SU00S020（メインメニュー）］の取得
	pRes = pComm->GetResponse();

	return true;
}

// マイナンバーカードを使用した受付システム認証処理
boolean loginETaxMyNumber(CLICommunicationPtr pComm, CLIHttpResponsePtr& pRes)
{
	_variant_t vtRes;
	CLIHttpRequestPtr pReq = NULL;

	//-------------------------------------------
	// 認証処理
	//-------------------------------------------
	// 実行（メインメニュー要求）
	pReq = pComm->CreateRequest("XU00S010");

	// リクエスト送信
	vtRes = pComm->Send();

	//-------------------------------------------
	// マイナンバーカードの読み取りへ
	pReq = pComm->CreateRequest("XU00S010_3");

	// リクエスト送信
	vtRes = pComm->Send();

	//-------------------------------------------
	//マイナンバーカードログイン（同期通信）
	vtRes = pComm->MyNumberCardLogin(NULL);

	// 正常にリクエストが処理された場合、SU00S020（メインメニュー）に画面遷移する
	if (vtRes != _variant_t("SU00S020")) {
		cerr << "受付システムの画面遷移が失敗しました：" << vtRes.bstrVal << endl;
		return false;
	}

	//-------------------------------------------
	// レスポンスデータ［SU00S020（メインメニュー）］の取得
	pRes = pComm->GetResponse();

	return true;
}


//=========================================================
int main(int argc, char* argv[])
{
	// 変数宣言
	CLICommunicationPtr pComm = NULL;
	CLIHttpResponsePtr pRes = NULL;

	HRESULT hr = S_OK;

	_bstr_t loginInfo;

	//------------------------------------------------------------
	// プログラムの開始
	//
	// COMの準備
	hr = CoInitialize(NULL);
	if (FAILED(hr)) {
		cerr << "COM+の準備でエラー: " << hr << endl;
		return -1;
	}

	// 利用者識別番号、パスワードを引数を使用するか
	string uid = USER_ID;
	string pwd = PASSWORD;
	if (argc == 3) {
		uid = argv[1];
		pwd = argv[2];
	}

	// 実行ファイルパスの取得（iniファイル、HTMLテンプレートファイルの格納先）
	char szPath[_MAX_PATH + 1] = {};
	char szDrive[_MAX_DRIVE + 1] = {};
	char szDir[_MAX_DIR + 1] = {};

	GetModuleFileName(NULL, szPath, _MAX_PATH);
	_splitpath_s(szPath, szDrive, sizeof(szDrive), szDir, sizeof(szDir), NULL, 0, NULL, 0);
	string basePath = szDrive;
	basePath += szDir;

	// 受付システム接続設定ファイル フルパス
	string iniFile = basePath.c_str();
	iniFile += "CLCommunication.ini";

	try {
		//------------------------------------------------------------
		// 送受信モジュール初期化
		boolean result = initCommModule(pComm, iniFile);
		if (!result) {
			// エラー
			cerr << "送受信モジュール：初期化エラー" << endl;

			pComm = NULL;
			CoUninitialize();

			return -1;
		}

		//------------------------------------------------------------
		// 受付システムログイン
		result = loginETax(pComm, uid, pwd, pRes);
		//result = loginETaxMyNumber(pComm, pRes);
		if (!result) {
			// エラー
			cerr << "送受信モジュール：接続エラー" << endl;

			pRes = NULL;
			pComm = NULL;
			CoUninitialize();

			return -1;
		}

		//------------------------------------------------------------
		// 引継ぎ情報の取得
		loginInfo = pRes->GetValue("引継ぎ情報");
		pRes = NULL;
	}
	catch (_com_error e) {
		cerr << "認証エラー : " << e.Error() << endl;
		pComm = NULL;
		CoUninitialize();

		return -1;
	}

	//------------------------------------------------------------
	// シームレスログインの実行

	// シームレスログイン GET用URL構築
	string url = "https://login.e-tax.nta.go.jp/login/reception/loginSeamless?oStHktgInf=";
	url += loginInfo;
	url += "&oStSeniSkGmnId=SM20S001";

	// 限定的URLエンコード（半角空白→%20に置き換え）
	const std::string strCharSpace = " ";
	const std::string strURLEncodeSpace = "%20";
	std::string::size_type pos = url.find(strCharSpace);
	while (pos != std::string::npos) {
		url.replace(pos, strCharSpace.length(), strURLEncodeSpace);
		pos = url.find(strCharSpace, pos + strURLEncodeSpace.length());
	}

	// 2.Webブラウザで生成したURLを開く
	HINSTANCE hInst = ShellExecute(NULL, "open", url.c_str(), NULL, NULL, SW_SHOWNORMAL);
	if (hInst <= (HINSTANCE)32) {
		// エラー
		cerr << "”HTMLファイルを開く”失敗 ：" << hInst << endl;
	}

	// 終了処理
	pComm = NULL;
	CoUninitialize();

	return 0;
}
