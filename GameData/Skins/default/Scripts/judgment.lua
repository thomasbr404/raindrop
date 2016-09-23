Judgment = {
	FadeoutTime = 0.5,
	FadeoutDuration = 0.15,
	Speed = 25,
	Tilt = 7, -- in degrees

	Scale = 0.20,
	ScaleHit = 0.08,
	ScaleMiss = 0.12,
	ScaleExtra = 0.1,

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


function Judgment:Init()
  print "Judgment Initializing."
	self.Atlas = TextureAtlas:skin_new("VSRG/judgment.csv")
  
  
  self.defaultX = self.Noteskin.GearWidth / 2 + self.Noteskin.GearStartX
  self.defaultY = ScreenHeight * 0.4
  print ("Judgment Default Pos: ", self.defaultX, self.defaultY)
  
	self.Object = ScreenObject {
		Layer = 24,
		Centered = 1,
		ScaleX = self.Scale,
		ScaleY = self.Scale,
		Texture = self.Atlas.File,
		Alpha = 0
	}
  
  if self.Position then 
    self.Object.X = self.Position.x
    self.Object.Y = self.Position.y
  else
    self.Object.X = self.defaultX
    self.Object.Y = self.defaultY  
  end
  print ("Judgment Real pos/Texture: ", self.Object.X, self.Object.Y, self.Object.Texture)

	self.LastAlternation = 0
	self.Time = self.FadeoutTime + self.FadeoutDuration

	self.IndicatorObject = ScreenObject {
		Texture = ("VSRG/" .. self.TimingIndicator),
		Layer = 24,
		Centered = 1,
		ScaleX = self.Scale,
		ScaleY = self.Scale,
		Alpha = 0,
		Y = self.Object.Y
	}
end

librd.make_new(Judgment, Judgment.Init)

function Judgment:GetComboLerp()
	local AAAThreshold = 8.0 / 9.0
	return clerp(self.Player.Combo,  -- cur
							 0, self.Player.Scorekeeper.MaxNotes * AAAThreshold, -- start end
							 0, 1) -- start val end val
end

function Judgment:Run(Delta)
	local ComboLerp = self:GetComboLerp()

	if Active ~= 0 then
		local AlphaRatio
		self.Time = self.Time + Delta

		local OldJudgeScale = self.Object.ScaleX
		local ScaleLerpAAA = ComboLerp * self.ScaleExtra
		local DeltaScale = (self.Scale + ScaleLerpAAA - OldJudgeScale) * Delta * self.Speed
		local FinalScale = max(0, OldJudgeScale + DeltaScale)

		self.Object:SetScale (FinalScale)

		if self.Time > self.FadeoutTime then
			local Time = self.Time - self.FadeoutTime
			if Time > 0 then
				local Ratio = Time / self.FadeoutDuration

				if Ratio < 1 then
					AlphaRatio = 1 - Ratio
				else
					AlphaRatio = 0
				end

			end
		else
			AlphaRatio = 1
		end

		local w = self.Object.Width
		local h = self.Object.Height

		self.Object.Alpha = (AlphaRatio)

		if self.LastAlternation == 0 then
			self.Object.Rotation = (self.Tilt * ComboLerp)
		else
			self.Object.Rotation = (-self.Tilt * ComboLerp)
		end

		if self.Value ~= 1 and self.ShowTimingIndicator == 1 then -- not a "flawless"

			self.IndicatorObject.Alpha = (AlphaRatio)

			local NewRatio = FinalScale / self.Scale
			self.IndicatorObject:SetScale(NewRatio)

			if self.EarlyOrLate == 1 then -- early
				self.IndicatorObject.X = self.Position.x - w/2 * FinalScale - 20
			else
				if self.EarlyOrLate == 2 then -- late
					self.IndicatorObject.X = self.Position.x + w/2 * FinalScale + 20
				end
			end
		else
			self.IndicatorObject.Alpha = 0
		end
	end
end

function Judgment:OnHit(JudgmentValue, Time, l, h, r, pn)
  if pn ~= self.Player.Number then
    return
  end

	self.Value = JudgmentValue

	if self.Value == 0 then
		self.Object.Lighten = (1)
		self.Object.LightenFactor = (1.5)
		self.Value = 1
	else
		self.Object.Lighten = 0
		self.Object.LightenFactor = 0
	end

	if self.LastAlternation == 0 then
		self.LastAlternation = 1
	else
		self.LastAlternation = 0
	end

	self.Atlas:SetObjectCrop(self.Object, self.Table[self.Value])
	self.Object.Height = self.Atlas.Sprites[self.Table[self.Value]].h
	self.Object.Width = self.Atlas.Sprites[self.Table[self.Value]].w

	if JudgmentValue ~= 5 then
		if JudgmentValue ~= -1 then
			local CLerp = self:GetComboLerp()
			self.Object:SetScale (self.ScaleHit + CLerp * self.ScaleExtra)
		else
			self.Object:SetScale (self.Scale)
		end
	else
		self.Object:SetScale (self.ScaleMiss)
	end

	self.Time = 0
	self.EarlyOrLate = EarlyOrLate
end

function Judgment:OnMiss(t, l, h, pn)
  self:OnHit(5, t, l, h, h, pn)
end