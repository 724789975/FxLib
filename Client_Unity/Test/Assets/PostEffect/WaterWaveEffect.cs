using UnityEngine;
 
public class WaterWaveEffect : PostEffectBase<WaterWaveEffect>
{
    //距离系数
    public float distanceFactor = 60.0f;
    //时间系数
    public float timeFactor = -30.0f;
    //sin函数结果系数
    public float totalFactor = 1.0f;
 
    //波纹宽度
    public float waveWidth = 0.3f;
    //波纹扩散的速度
    public float waveSpeed = 0.3f;
 
	[SerializeField]
    private float waveStartTime = 0;
	[SerializeField]
    private Vector4 startPos = new Vector4(0.5f, 0.5f, 0, 0);

	public bool bPlayAtStart = false;

	void Awake()
	{
		CreateInstance(this);
	}

	void Start()
	{
		if (shader == null)
		{
			enabled = false;
			return;
		}
		if (shader.isSupported == false)
		{
			enabled = false;
			return;
		}
		if (bPlayAtStart)
		{
			waveStartTime = Time.time;
		}
	}

	void OnRenderImage (RenderTexture source, RenderTexture destination)
    {
        //计算波纹移动的距离，根据enable到目前的时间*速度求解
        float curWaveDistance = (Time.time - waveStartTime) * waveSpeed;
        //设置一系列参数
        _Material.SetFloat("_distanceFactor", distanceFactor);
        _Material.SetFloat("_timeFactor", timeFactor);
        _Material.SetFloat("_totalFactor", totalFactor);
        _Material.SetFloat("_waveWidth", waveWidth);
        _Material.SetFloat("_curWaveDis", curWaveDistance);
        _Material.SetVector("_startPos", startPos);
		Graphics.Blit (source, destination, _Material);
	}
 
    void Update()
    {
		//if (Input.GetMouseButton(0))
		//{
		//	Vector2 mousePos = Input.mousePosition;
		//	//将mousePos转化为（0，1）区间
		//	startPos = new Vector4(mousePos.x / Screen.width, mousePos.y / Screen.height, 0, 0);
		//	waveStartTime = Time.time;
		//}
	}
}

