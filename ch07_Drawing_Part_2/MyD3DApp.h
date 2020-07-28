#pragma once

#include "../Common/d3dApp.h"
#include "../Common/MathHelper.h"
#include "../Common/UploadBuffer.h"
#include "../Common/GeometryGenerator.h"
#include "FrameResource.h"

using Microsoft::WRL::ComPtr;
using namespace DirectX;

struct RenderItem
{
	RenderItem() = default;

	XMFLOAT4X4 World = MathHelper::Identity4x4();


	int NumFramesDirty = gNumFrameResources;

	UINT ObjCBIndex = -1;

	MeshGeometry* Geo = nullptr;


	D3D12_PRIMITIVE_TOPOLOGY PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;


	UINT IndexCount = 0;
	UINT StartIndexLocation = 0;
	int BaseVertexLocation = 0;
};

class MyD3DApp : public D3DApp
{
private:
	// 프레임 자원들에 대한 정보를 저장하자
	static const int m_nMaxFrameResourceIdx = 3;
	std::vector<std::unique_ptr<FrameResource>> m_vecFrameResources;
	FrameResource* m_pCurFrameResource = nullptr;
	UINT m_nCurFrameResourceIdx = 0;

	// 렌더링할 모든 원소를 저장한다.
	std::vector<std::unique_ptr<RenderItem>> m_vecRenderItems;

	ComPtr<ID3D12RootSignature> m_RootSignature = nullptr;
	ComPtr<ID3D12DescriptorHeap> m_CbvHeap = nullptr;

	std::unique_ptr<MeshGeometry> m_Pyramid = nullptr;
	std::unique_ptr<MeshGeometry> m_BoxGeo = nullptr;

	ComPtr<ID3DBlob> m_vsByteCode = nullptr;
	ComPtr<ID3DBlob> m_psByteCode = nullptr;

	std::vector<D3D12_INPUT_ELEMENT_DESC> m_InputLayout;

	ComPtr<ID3D12PipelineState> m_PSO = nullptr;

	XMFLOAT4X4 m_World = MathHelper::Identity4x4();
	XMFLOAT4X4 m_View = MathHelper::Identity4x4();
	XMFLOAT4X4 m_Proj = MathHelper::Identity4x4();

	float m_Theta = 1.5f * XM_PI;
	float m_Phi = XM_PIDIV4;
	float m_Radius = 5.0f;

	POINT m_LastMousePos;

private:
	// 짜피 Run 메소드 아래서 돌아가는 것들이라
	// private로 코드 상의 직접 접근을 막는다.
	virtual void OnResize() override;
	virtual void Update(const GameTimer& gt) override;
	virtual void Draw(const GameTimer& gt) override;

	virtual void OnMouseDown(WPARAM btnState, int x, int y)override;
	virtual void OnMouseUp(WPARAM btnState, int x, int y)override;
	virtual void OnMouseMove(WPARAM btnState, int x, int y)override;

	/* 버퍼와 셰이더 관련 메소드들 */
	void BuildFrameResources();
	void BuildDescriptorHeaps();
	void BuildConstantBuffers();
	void BuildRootSignature();
	void BuildShadersAndInputLayout();
	void BuildBoxGeometry();
	void BuildPSO();

private : 
	// 쓸모없는 생성자들
	MyD3DApp(const MyD3DApp& rhs) = delete;
	MyD3DApp& operator=(const MyD3DApp& rhs) = delete;

public:
	MyD3DApp(HINSTANCE hInstance);
	~MyD3DApp();

public:
	virtual bool Initialize() override;
};

