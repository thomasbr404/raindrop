

Lifebar = {
	FillSize = 367,
	MarginFile = "VSRG/stage-lifeb.png",
	FillFile = "VSRG/stage-lifeb-s.png",
	Width = 50,
	Speed = 10
}

Judgment = {

	FadeoutTime = 1,
	FadeoutDuration = 0.25,
	Speed = 15,
	Tilt = 7, -- in degrees

	Scale = 0.20,
	ScaleHit = 0.30,
	ScaleMiss = 0.12,

	Position = {
		x = GearWidth/2 + GearStartX,
		y = ComboDisplay.Position.y + ComboDisplay.DigitHeight + 20
	},

	Table = {
		"judge-excellent.png",
		"judge-perfect.png",
		"judge-good.png",
		"judge-bad.png",
		"judge-miss.png",
	},

	TimingIndicator = "hiterror.png",
	ShowTimingIndicator = 1
}

function Lifebar.Init()

	Lifebar.Margin = Engine:CreateObject()
	Lifebar.Fill = Engine:CreateObject()
	Lifebar.Fill2 = Engine:CreateObject()

	Lifebar.Margin.Image = (Lifebar.MarginFile)
	Lifebar.Margin.Layer = 25
	Lifebar.Margin.Centered = 1

	local w = Lifebar.Margin.Width
	local h = Lifebar.Margin.Height
	

	Lifebar.Position = { 
		x = GearStartX + GearWidth + 42,
		y = ScreenHeight - h / 2
	}

	Lifebar.CurrentPosition = {
		x = Lifebar.Position.x,
		y = ScreenHeight
	}

	Lifebar.Margin.X = Lifebar.CurrentPosition.x
	Lifebar.Margin.Y = Lifebar.Position.y
	
	Lifebar.Fill.Image = Lifebar.FillFile
	Lifebar.Fill.Width = Lifebar.Width
	Lifebar.Fill.Height = Lifebar.FillSize
	Lifebar.Fill.Layer = 26
	Lifebar.Fill.Centered = 1

	Lifebar.Fill2.Image = Lifebar.FillFile
	Lifebar.Fill2.Layer = 26
	Lifebar.Fill2.Centered = 1
	Lifebar.Fill2.BlendMode = BlendAdd

	Lifebar.Display = 0
end

function Lifebar.Cleanup()
	Obj.CleanTarget(Lifebar.Margin)
	Obj.CleanTarget(Lifebar.Fill)
	Obj.CleanTarget(Lifebar.Fill2)
end

function Lifebar.Run(Delta)
	local DeltaLifebar = (LifebarValue - Lifebar.Display)
	local DP = 1 - (Beat - math.floor(Beat))

	Lifebar.Display = DeltaLifebar * Delta * Lifebar.Speed + Lifebar.Display

	local partA = Lifebar.Display * 0.98
	local partB = Lifebar.Display * 0.02 * DP
	local Display = partA + partB
	local NewY = ScreenHeight - Lifebar.FillSize * (Display) / 2
	local NewYFixed = ScreenHeight - Lifebar.FillSize * (Lifebar.Display) / 2

	Lifebar.CurrentPosition = Lifebar.Position

	Lifebar.Fill.ScaleY = Lifebar.Display 
	Lifebar.Fill.X = Lifebar.Position.x
	Lifebar.Fill.Y = NewYFixed
	Lifebar.Fill:SetCropByPixels( 0, Lifebar.Width, Lifebar.FillSize - Lifebar.FillSize * Lifebar.Display, Lifebar.FillSize )

	Lifebar.Fill2.ScaleY = Display 
	Lifebar.Fill2.X = Lifebar.Position.x
	Lifebar.Fill2.Y = NewY
	Lifebar.Fill2:SetCropByPixels( 0, Lifebar.Width, Lifebar.FillSize - Lifebar.FillSize * Display, Lifebar.FillSize )
	Lifebar.Fill2.Alpha = ( DP * LifebarValue )
end

function Judgment.Init()
	Judgment.Object = Engine:CreateObject()
	
	Judgment.Object.Layer = 24
	Judgment.Object.Centered = 1
	Judgment.Object:SetScale(Judgment.Scale)
	Judgment.Object.X = Judgment.Position.x
	Judgment.Object.Y = Judgment.Position.y

	Judgment.LastAlternation = 0
	Judgment.Time = Judgment.FadeoutTime

	Judgment.IndicatorObject = Engine:CreateObject()
	
	Judgment.IndicatorObject.Image = ("VSRG/" .. Judgment.TimingIndicator)
	Judgment.IndicatorObject.Layer = 24
	Judgment.IndicatorObject.Centered = 1
	Judgment.IndicatorObject:SetScale(Judgment.Scale)
	Judgment.IndicatorObject.Alpha = 0
end

function Judgment.Run(Delta)
	local ComboPercent = ScoreKeeper:getScore(ST_COMBO) / ScoreKeeper:getMaxNotes()
	local AAAThreshold = 8.0 / 9.0
	local ComboLerp = math.min(ComboPercent, AAAThreshold) / AAAThreshold
	
	if Active ~= 0 then
		local AlphaRatio
		Judgment.Time = Judgment.Time + Delta

		local OldJudgeScale = Judgment.Object.ScaleX

		local DeltaScale = (Judgment.Scale - OldJudgeScale) * Delta * Judgment.Speed
		local FinalScale = OldJudgeScale + DeltaScale

		Judgment.Object:SetScale (FinalScale)

		if Judgment.Time > Judgment.FadeoutTime then
			local Time = Judgment.Time - Judgment.FadeoutTime
			if Time > 0 then
				local Ratio = Time / Judgment.FadeoutDuration

				if Ratio < 1 then
					AlphaRatio = 1 - Ratio
				else
					AlphaRatio = 0
				end

			end
		else
			AlphaRatio = 1
		end
		
		local w = Judgment.Object.Width
		local h = Judgment.Object.Height

		Judgment.Object.Alpha = (AlphaRatio)
				
		if Judgment.LastAlternation == 0 then
			Judgment.Object.Rotation = (Judgment.Tilt * ComboLerp)
		else
			Judgment.Object.Rotation = (-Judgment.Tilt * ComboLerp)
		end

		if Judgment.Value ~= 1 and Judgment.ShowTimingIndicator == 1 then -- not a "flawless"
			
			Judgment.IndicatorObject.Alpha = (AlphaRatio)

			local NewRatio = FinalScale / Judgment.Scale
			Judgment.IndicatorObject:SetScale(NewRatio)

			if Judgment.EarlyOrLate == 1 then -- early
				Judgment.IndicatorObject.X = Judgment.Position.x - w/2 * FinalScale - 20
			else 
				if Judgment.EarlyOrLate == 2 then -- late
					Judgment.IndicatorObject.X = Judgment.Position.x + w/2 * FinalScale + 20
				end
			end
		else
			Judgment.IndicatorObject.Alpha = 0
		end
	end
end

function Judgment.Hit(JudgmentValue, EarlyOrLate)

	Judgment.Value = JudgmentValue

	if Judgment.Value == 0 then
		Judgment.Object.Lighten = (1)
		Judgment.Object.LightenFactor = (1.5)
		Judgment.Value = 1
	else
		Judgment.Object.Lighten = 0
	end

	if Judgment.LastAlternation == 0 then
		Judgment.LastAlternation = 1
	else 
		Judgment.LastAlternation = 0
	end
	
	Judgment.Object.Image = ("VSRG/" .. Judgment.Table[Judgment.Value])
	
	if JudgmentValue ~= 5 then
		if JudgmentValue ~= -1 then
			Judgment.Object:SetScale (Judgment.ScaleHit)
		else
			Judgment.Object:SetScale (Judgment.Scale)
		end
	else
		Judgment.Object:SetScale (Judgment.ScaleMiss)
	end

	Judgment.Time = 0
	Judgment.EarlyOrLate = EarlyOrLate
end
