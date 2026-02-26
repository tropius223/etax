// TestCommVC01.cpp : コンソール アプリケーション用のエントリ ポイントの定義
//

#include "stdafx.h"
#include "iostream"

// 送受信モジュール
#import ".\bin\CLCommunication.dll"

// 使用する名前空間
using namespace std;
using namespace CLCOMMUNICATIONLib;		// 送受信モジュール

// エラー表示用
#define	CREATE_REQUEST		"CreateRequest"
#define	SEND				"Send"
#define	GET_HTTPSTATUS_CODE	"GetHttpStatusCode"
#define	GET_RESPONSE		"GetResponse"
#define	GET_ERRORRESPONSE	"GetErrorResponse"
#define	SET_PARAMETER		"SetParameter"

#define USER_ID		"1111111111111"
#define PASSWORD	"Pass0001"

//=========================================================
// TestCommVC01
//
// 概要：e-taxソフトの送受信モジュールの簡単なサンプル
// 説明：このモジュールでは、利用者識別番号と暗証番号を
//       指定して認証処理を行い、次いでメッセージボックス
//       から最初のメッセージを取り出し、タイトルを表示
//       する例です。
//=========================================================
int main(int argc, char* argv[])
{
	// 変数宣言
	CLICommunicationPtr pComm = NULL;
	CLIHttpRequestPtr pReq = NULL;
	CLIHttpResponsePtr pRes = NULL;

	_bstr_t bstrMessage;
	char szMethod[64];

	int ret = 0;
	HRESULT hr = S_OK;

	VARIANT vtIndex;
	VARIANT vtRes;
	long nStatus = -1L;
	
	_bstr_t bstrIniFile;
	char szPath[_MAX_PATH+1];
	char szDrive[_MAX_DRIVE+1];
	char szDir[_MAX_DIR+1];


	//------------------------------------------------------------
	// プログラムの開始
	//

	// 変数の初期化
	VariantInit(&vtIndex);
	VariantInit(&vtRes);

	// INIファイルはEXEの一つ上
	ZeroMemory(szPath, _MAX_PATH+1);
	ZeroMemory(szDrive, _MAX_DRIVE+1);
	ZeroMemory(szDir, _MAX_DIR+1);
	GetModuleFileName(NULL, szPath, _MAX_PATH);
	_splitpath(szPath, szDrive, szDir, NULL, NULL);
	bstrIniFile = szDrive;
	bstrIniFile += szDir;
	bstrIniFile += "\\..\\";
	bstrIniFile += "CLCommunication.ini";	// フルパス

	// COM+の準備
	hr = CoInitialize(NULL);
	if(FAILED(hr)){
		cerr << "COM+の準備でエラー: " << hr << endl;
		ret = 1;
		goto TERMINATE;
	}

	// CLCommunicationインターフェイスの作成
	hr = pComm.CreateInstance(__uuidof(CLCCommunication));
	if(FAILED(hr)){
		cerr << "CLCommunicationインターフェイスの作成でエラー: " << hr << endl;
		ret = 2;
		goto TERMINATE;
	}

	// 受付システムへのLogin認証とメインメニューからのリンク先を設定
	hr = pComm->put_AttestationUrl(bstrIniFile);
	if(FAILED(hr)){
		cerr << "受付システムへのLogin認証とメインメニューからのリンク先の設定でエラー: " <<
			hr << endl;
		ret = 3;
		goto TERMINATE;
	}

	//-------------------------------------------
	// メインメニューを要求（認証）
	//-------------------------------------------
	try{
		ZeroMemory(szMethod, sizeof(szMethod));
		CopyMemory(szMethod, CREATE_REQUEST, sizeof(CREATE_REQUEST));
		pReq = pComm->CreateRequest("XU00S010");	// 認証（メインメニューの要求）
		
		ZeroMemory(szMethod, sizeof(szMethod));
		CopyMemory(szMethod, SET_PARAMETER, sizeof(SET_PARAMETER));

		//========================================
		// 必要に応じてここのパラメータは変更する
		// （ここでは適当な値を使用。
		//  試す場合は適正な値に変えてください）
		pReq->SetParameter("sUserId", USER_ID);		// 利用者識別番号

		try{
			vtRes = pComm->Send();	// 送信
		}
		catch(_com_error e){ }

		ZeroMemory(szMethod, sizeof(szMethod));
		CopyMemory(szMethod, CREATE_REQUEST, sizeof(CREATE_REQUEST));
		pReq = pComm->CreateRequest("XU00S010_1");	// 認証実行
		
		ZeroMemory(szMethod, sizeof(szMethod));
		CopyMemory(szMethod, SET_PARAMETER, sizeof(SET_PARAMETER));

		//========================================
		// 必要に応じてここのパラメータは変更する
		// （ここでは適当な値を使用。
		//  試す場合は適正な値に変えてください）
		pReq->SetParameter("sUserId", USER_ID);		// 利用者識別番号
		pReq->SetParameter("sPwd", PASSWORD);		// 暗証番号

		try{
			vtRes = pComm->Send();	// 送信
		}
		catch(_com_error e){ }

		ZeroMemory(szMethod, sizeof(szMethod));
		CopyMemory(szMethod, GET_HTTPSTATUS_CODE, sizeof(GET_HTTPSTATUS_CODE));
		nStatus = pComm->GetHttpStatusCode();
		if(nStatus != 200){		// 認証失敗
			cerr << "認証エラー at Send; HTTP Status = " << nStatus << endl;
			ZeroMemory(szMethod, sizeof(szMethod));
			CopyMemory(szMethod, GET_ERRORRESPONSE, sizeof(GET_ERRORRESPONSE));
			pRes = pComm->GetErrorResponse();
			if(pRes){
				bstrMessage = pRes->XML;
				cerr << (char*)bstrMessage << endl;
			}
			else{
				cerr << "受付システムから正しいデータが取得できませんでした" << endl;
			}
			ret = 5;
			goto TERMINATE;
		}
	}
	catch(_com_error e){
		cerr << "認証エラー at " << szMethod << ": " << e.Error() << endl;
		ret = 4;
		goto TERMINATE;
	}
	cout << "認証成功" << endl;

	//-------------------------------------------
	// メインメニュー → メッセージボックス
	//-------------------------------------------
	try{
		ZeroMemory(szMethod, sizeof(szMethod));
		CopyMemory(szMethod, CREATE_REQUEST, sizeof(CREATE_REQUEST));
		pReq = pComm->CreateRequest("XU00S020_BUSINESS_2");		// メッセージボックス

		ZeroMemory(szMethod, sizeof(szMethod));
		CopyMemory(szMethod, SEND, sizeof(SEND));
		try{
			vtRes = pComm->Send();
		}
		catch(_com_error e){ }

		ZeroMemory(szMethod, sizeof(szMethod));
		CopyMemory(szMethod, GET_HTTPSTATUS_CODE, sizeof(GET_HTTPSTATUS_CODE));
		nStatus = pComm->GetHttpStatusCode();
		if(nStatus != 200){		// メッセージボックス取得失敗
			cerr << "メッセージボックス取得エラー at Send; HTTP Status = " << nStatus << endl;
			ZeroMemory(szMethod, sizeof(szMethod));
			CopyMemory(szMethod, GET_ERRORRESPONSE, sizeof(GET_ERRORRESPONSE));
			pRes = pComm->GetErrorResponse();
			if(pRes){
				bstrMessage = pRes->XML;
				cerr << (char*)bstrMessage << endl;
			}
			else{
				cerr << "受付システムから正しいデータが取得できませんでした" << endl;
			}
			ret = 5;
			goto TERMINATE;
		}
	}
	catch(_com_error e){
		cerr << "メッセージボックス取得エラー at " << szMethod << ": " << e.Error() << endl;
		ret = 4;
		goto TERMINATE;
	}
	cout << "メッセージボックス取得成功" << endl;

	//-------------------------------------------
	// メッセージ詳細
	//-------------------------------------------
	try{
		// レスポンスを取得
		ZeroMemory(szMethod, sizeof(szMethod));
		CopyMemory(szMethod, GET_RESPONSE, sizeof(GET_RESPONSE));
		pRes = pComm->GetResponse();

		// 要求を作成
		ZeroMemory(szMethod, sizeof(szMethod));
		CopyMemory(szMethod, CREATE_REQUEST, sizeof(CREATE_REQUEST));
		pReq = pComm->CreateRequest("XU00S090");		// （メッセージ）詳細表示

		//========================================
		// 必要に応じてここのパラメータは変更する
		// （今回は１番目のメールの詳細を取得）
		vtIndex.vt = VT_I4;
		vtIndex.lVal = 1L;	// とりあえず１番目のメール
		pReq->SetParameter("objSel", pRes->GetValue("内容_メール通番", vtIndex));

		ZeroMemory(szMethod, sizeof(szMethod));
		CopyMemory(szMethod, SEND, sizeof(SEND));
		try{
			vtRes = pComm->Send();
		}
		catch(_com_error e){ }

		// 結果のタイトルのみをとりあえず表示
		ZeroMemory(szMethod, sizeof(szMethod));
		CopyMemory(szMethod, GET_HTTPSTATUS_CODE, sizeof(GET_HTTPSTATUS_CODE));
		nStatus = pComm->GetHttpStatusCode();
		if(nStatus == 200){
			ZeroMemory(szMethod, sizeof(szMethod));
			CopyMemory(szMethod, GET_RESPONSE, sizeof(GET_RESPONSE));
			pRes = pComm->GetResponse();
			bstrMessage = pRes->GetValue("タイトル");
		}
		else{
			ZeroMemory(szMethod, sizeof(szMethod));
			CopyMemory(szMethod, GET_ERRORRESPONSE, sizeof(GET_ERRORRESPONSE));
			pRes = pComm->GetErrorResponse();
			if(pRes){
				bstrMessage = pRes->XML;
			}
			else{
				bstrMessage = "受付システムから正しいデータが取得できませんでした";
			}
		}
		cout << (char*)bstrMessage << endl << endl;
		BSTR bsXml;
		pRes->get_XML(&bsXml);
		_bstr_t bsXmlVal = bsXml;
		cout << (char*)bsXmlVal << endl << endl;
	}
	catch(_com_error e){
		cerr << "メッセージ詳細表示 at " << szMethod << ": " << e.Error() << endl;
		ret = 4;
		goto TERMINATE;
	}

// 共通終了処理（各種オブジェクトの解放等）
TERMINATE:
	pComm = NULL;
	pReq = NULL;
	pRes = NULL;
	CoUninitialize();
	return 0;
}
