#pragma once

#include "../Common/d3dApp.h"
#include "../Common/MathHelper.h"
#include "../Common/UploadBuffer.h"

using Microsoft::WRL::ComPtr;
using namespace DirectX;
using namespace DirectX::PackedVector;

struct Vertex
{
	XMFLOAT3 position;
	XMFLOAT4 color;
};

struct ObjectConstants
{
	XMFLOAT4X4 WorldViewProj = MathHelper::Identity4x4();
};

class MyD3DBoxApp : public D3DApp
{
private:
	ComPtr<ID3D12RootSignature> mRootSignature = nullptr;
	ComPtr<ID3D12DescriptorHeap> mCbvHeap = nullptr;

	std::unique_ptr<UploadBuffer<ObjectConstants>> mObjectCB = nullptr;

	std::unique_ptr<MeshGeometry> mBoxGeo = nullptr;

	ComPtr<ID3DBlob> mvsByteCode = nullptr;
	ComPtr<ID3DBlob> mpsByteCode = nullptr;

	std::vector<D3D12_INPUT_ELEMENT_DESC> mInputLayout;

	ComPtr<ID3D12PipelineState> mPSO = nullptr;

	XMFLOAT4X4 mWorld = MathHelper::Identity4x4();
	XMFLOAT4X4 mView = MathHelper::Identity4x4();
	XMFLOAT4X4 mProj = MathHelper::Identity4x4();

	float mTheta = 1.5f * XM_PI;
	float mPhi = XM_PIDIV4;
	float mRadius = 5.0f;

	POINT mLastMousePos;

private:
	// ¥�� Run �޼ҵ� �Ʒ��� ���ư��� �͵��̶�
	// private�� �ڵ� ���� ���� ������ ���´�.
	virtual void OnResize() override;
	virtual void Update(const GameTimer& gt) override;
	virtual void Draw(const GameTimer& gt) override;

	virtual void OnMouseDown(WPARAM btnState, int x, int y)override;
	virtual void OnMouseUp(WPARAM btnState, int x, int y)override;
	virtual void OnMouseMove(WPARAM btnState, int x, int y)override;

	/* ���ۿ� ���̴� ���� �޼ҵ�� */
	void BuildDescriptorHeaps();
	void BuildConstantBuffers();
	void BuildRootSignature();
	void BuildShadersAndInputLayout();
	void BuildBoxGeometry();
	void BuildPSO();

private : 
	// ������� �����ڵ�
	MyD3DBoxApp(const MyD3DBoxApp& rhs) = delete;
	MyD3DBoxApp& operator=(const MyD3DBoxApp& rhs) = delete;

public:
	MyD3DBoxApp(HINSTANCE hInstance);
	~MyD3DBoxApp();

public:
	virtual bool Initialize() override;
};

