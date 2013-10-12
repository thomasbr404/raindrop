#include "Global.h"
#include "ScreenEdit.h"
#include "GraphicsManager.h"
#include "ImageLoader.h"
#include "Audio.h"
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>



ScreenEdit::ScreenEdit(IScreen *Parent)
	: ScreenGameplay(Parent)
{
	ShouldChangeScreenAtEnd = false; // So it doesn't go into screen evaluation.
	GuiInitialized = false;
	CurrentFraction = 0;
	Measure = 0;
	EditScreenState = Editing;

	GhostObject.SetImage(ImageLoader::LoadSkin("hitcircle.png"));
	GhostObject.Alpha = 0.7f;
	GhostObject.Centered = true;
	GhostObject.SetSize(CircleSize);

	EditInfo.LoadSkinFontImage("font.tga", glm::vec2(18, 32), glm::vec2(34,34), glm::vec2(10,16), 32);
	EditMode = true;
	HeldObject = NULL;
	Mode = Select;
}

void ScreenEdit::Cleanup()
{
	// remove our filthy windows
	CEGUI::WindowManager::getSingleton().destroyWindow("ScreenEditRoot");
}

void ScreenEdit::Init(Song *Other)
{
	if (Other != NULL)
	{
		if (Other->Difficulties.size() == 0) // No difficulties? Create a new one.
			Other->Difficulties.push_back(new SongInternal::Difficulty());

		ScreenGameplay::Init (Other, 0);
		NotesInMeasure.clear(); 

		Other->Difficulties[0]->Timing.push_back(SongInternal::Difficulty::TimingSegment());
	}

	if (GuiInitialized)
		return;

	// CEGUI stuff ahead...
	using namespace CEGUI;

	WindowManager& winMgr = WindowManager::getSingleton();
	root = (DefaultWindow*)winMgr.createWindow("DefaultWindow", "ScreenEditRoot");

	System::getSingleton().setGUISheet(root);
	
	FrameWindow* fWnd = static_cast<FrameWindow*>(winMgr.createWindow( "TaharezLook/FrameWindow", "screenEditWindow" ));
	fWnd->setPosition(UVector2 (cegui_reldim(0.f), cegui_reldim(0.f)));
	fWnd->setSize(UVector2 (cegui_reldim(0.25f), cegui_reldim(0.85f)));
	root->addChildWindow(fWnd);

	Window* st = winMgr.createWindow("TaharezLook/StaticText", "BPMText");
    fWnd->addChildWindow(st);
	st->setText("BPM");
	st->setPosition(UVector2(cegui_reldim(0.1f), cegui_reldim(0.04f)));
	st->setSize(UVector2(cegui_reldim(0.3f), cegui_reldim(0.04f)));

	BPMBox = static_cast<Editbox*> (winMgr.createWindow("TaharezLook/Editbox", "BPMBox"));
	BPMBox->setSize(UVector2 (cegui_reldim(0.8f), cegui_reldim(0.15f)));
	BPMBox->setPosition(UVector2 (cegui_reldim(0.1f), cegui_reldim(0.07f)));
	fWnd->addChildWindow(BPMBox);

	Window* st2 = winMgr.createWindow("TaharezLook/StaticText", "MsrText");
    fWnd->addChildWindow(st2);
	st2->setText("Measure");
	st2->setPosition(UVector2(cegui_reldim(0.1f), cegui_reldim(0.5f)));
	st2->setSize(UVector2(cegui_reldim(0.5f), cegui_reldim(0.04f)));

	Window* st3 = winMgr.createWindow("TaharezLook/StaticText", "FraText");
    fWnd->addChildWindow(st3);
	st3->setText("Fraction");
	st3->setPosition(UVector2(cegui_reldim(0.1f), cegui_reldim(0.3f)));
	st3->setSize(UVector2(cegui_reldim(0.5f), cegui_reldim(0.04f)));

	Window* st4 = winMgr.createWindow("TaharezLook/StaticText", "OffsText");
    fWnd->addChildWindow(st4);
	st4->setText("Offset");
	st4->setPosition(UVector2(cegui_reldim(0.1f), cegui_reldim(0.7f)));
	st4->setSize(UVector2(cegui_reldim(0.3f), cegui_reldim(0.04f)));

	CurrentMeasure = static_cast<Editbox*> (winMgr.createWindow("TaharezLook/Editbox", "MeasureBox"));
	CurrentMeasure->setSize(UVector2 (cegui_reldim(0.8f), cegui_reldim(0.1f)));
	CurrentMeasure->setPosition(UVector2 (cegui_reldim(0.1f), cegui_reldim(0.4f)));

	fWnd->addChildWindow(CurrentMeasure);

	OffsetBox = static_cast<Editbox*> (winMgr.createWindow("TaharezLook/Editbox", "OffsetBox"));
	OffsetBox->setSize(UVector2 (cegui_reldim(0.8f), cegui_reldim(0.1f)));
	OffsetBox->setPosition(UVector2 (cegui_reldim(0.1f), cegui_reldim(0.6f)));
	fWnd->addChildWindow(OffsetBox);

	FracBox = static_cast<Editbox*> (winMgr.createWindow("TaharezLook/Editbox", "FracBox"));
	FracBox->setSize(UVector2 (cegui_reldim(0.8f), cegui_reldim(0.05f)));
	FracBox->setPosition(UVector2 (cegui_reldim(0.1f), cegui_reldim(0.25f)));
	fWnd->addChildWindow(FracBox);
	
	BPMBox->setText( (boost::format("%d\n") % CurrentDiff->Timing[0].Value).str().c_str() );
	OffsetBox->setText( (boost::format("%d\n") % CurrentDiff->Offset).str().c_str() );
	FracBox->setText("0");
	CurrentMeasure->setText("0");

	winMgr.getWindow("MeasureBox")->
		subscribeEvent(Editbox::EventTextChanged, Event::Subscriber(&ScreenEdit::measureTextChanged, this));

	winMgr.getWindow("BPMBox")->
		subscribeEvent(Editbox::EventTextChanged, Event::Subscriber(&ScreenEdit::bpmTextChanged, this));

	winMgr.getWindow("OffsetBox")->
		subscribeEvent(Editbox::EventTextChanged, Event::Subscriber(&ScreenEdit::offsetTextChanged, this));

	winMgr.getWindow("FracBox")->
		subscribeEvent(Editbox::EventTextChanged, Event::Subscriber(&ScreenEdit::fracTextChanged, this));

	GuiInitialized = true;
}

void ScreenEdit::StartPlaying( int32 _Measure )
{
	ScreenGameplay::Init(MySong, 0);
	Measure = _Measure;
	seekTime( spb(CurrentDiff->Timing[0].Value) * Measure * 4 + CurrentDiff->Offset);
	savedMeasure = Measure;
}

void ScreenEdit::HandleInput(int32 key, int32 code, bool isMouseInput)
{
	if (EditScreenState == Playing)
		ScreenGameplay::HandleInput(key, code, isMouseInput);

	if (!isMouseInput)
	{
		if (key == 'P') // pressed p?
		{
			MySong->Process(false);
			if (EditScreenState == Editing) // if we're editing, start playing the song
			{
				EditScreenState = Playing;
				StartPlaying( Measure );
			} 
			else if (EditScreenState == Playing) // if we're playing, go back to editing
			{
				EditScreenState = Editing;
				Measure = savedMeasure;
				stopMusic();
				RemoveTrash();
			}
		}

		if (EditScreenState != Playing)
		{
			if (code == GLFW_PRESS)
			{
				if (key == GLFW_KEY_RIGHT)
				{
					if (HeldObject)
					{
						HeldObject->hold_duration += 4.0 / (float)CurrentDiff->Measures[Measure].Fraction;
						MySong->Process(false);
					}

					CurrentFraction++;

					if (CurrentFraction >= CurrentDiff->Measures[Measure].Fraction)
					{
						CurrentFraction = 0;
						if (Measure+1 < CurrentDiff->Measures.size()) // Advance a measure
							Measure++;
					}

				}else if (key == GLFW_KEY_LEFT)
				{
					if (HeldObject)
					{
						HeldObject->hold_duration -= 4.0 / (float)CurrentDiff->Measures[Measure].Fraction;
						if (HeldObject->hold_duration < 0)
							HeldObject->hold_duration = 0;
						MySong->Process(false);
					}

					if (CurrentFraction || Measure > 1)
					{
						CurrentFraction--;

						if (CurrentFraction > CurrentDiff->Measures[Measure].Fraction) // overflow
						{
							CurrentFraction = CurrentDiff->Measures[Measure].Fraction-1;

							if (Measure > 0) // Go back a measure
								Measure--;
						}
					}
				}else if (key == 'S') // Save!
				{
					std::string DefaultPath = "chart.dcf";
					MySong->Repack();
					MySong->Save((MySong->SongDirectory + std::string("/") + DefaultPath).c_str());
					MySong->Process();
				}else if (key == 'Q')
				{
					if (Mode == Select)
						Mode = Normal;
					else if (Mode == Normal)
						Mode = Hold;
					else
						Mode = Select;
				}else if (key == 'R') // Repeat previous measure's fraction
				{
					if (Measure > 0)
						AssignFraction(Measure, CurrentDiff->Measures.at(Measure-1).Fraction);
				}else if (key == 'T') // Insert another measure, go to it and restart fraction
				{
					CurrentDiff->Measures.resize(CurrentDiff->Measures.size()+1);
					Measure = CurrentDiff->Measures.size()-1;
					CurrentFraction = 0;
					AssignFraction(Measure, CurrentDiff->Measures[Measure-1].Fraction);
				}else if (key == GLFW_KEY_ESC)
				{
					Running = false; // Get out.
				}else if (key == GLFW_KEY_F1)
				{
					root->setVisible(!root->isVisible());
				}else if (key == 'X')
				{
					Measure++;
				}else if (key == 'Z')
				{
					if (Measure > 0)
						Measure--;
				}else if (key == GLFW_KEY_F8)
				{
					IsAutoplaying = !IsAutoplaying;
				}
			}
		}
	}else // mouse input
	{
		if (EditScreenState == Editing)
		{
			if (Mode != Select && code == GLFW_PRESS)
			{
				if (Measure >= 0 && Measure < CurrentDiff->Measures.size())
				{
					if (CurrentDiff->Measures.at(Measure).MeasureNotes.size() > CurrentFraction)
					{
						if (key == GLFW_MOUSE_BUTTON_LEFT)
						{
							if (Mode == Normal)
							{
								CurrentDiff->Measures.at(Measure).MeasureNotes.at(CurrentFraction).SetPositionX(GhostObject.GetPosition().x);
								CurrentDiff->Measures.at(Measure).MeasureNotes.at(CurrentFraction).MeasurePos = CurrentFraction;
							}else if (Mode == Hold)
							{
								HeldObject = &CurrentDiff->Measures.at(Measure).MeasureNotes.at(CurrentFraction);
								HeldObject->SetPositionX(GhostObject.GetPosition().x);
								HeldObject->MeasurePos = CurrentFraction;
								HeldObject->endTime = 0;
								HeldObject->hold_duration = 0;
							}
						}else
						{
							HeldObject = NULL;
							CurrentDiff->Measures.at(Measure).MeasureNotes.at(CurrentFraction).SetPositionX(0);
							CurrentDiff->Measures.at(Measure).MeasureNotes.at(CurrentFraction).endTime = 0;
							CurrentDiff->Measures.at(Measure).MeasureNotes.at(CurrentFraction).hold_duration = 0;
						}

						MySong->Process(false);
					}
				}
			}

			if (Mode == Hold && code == GLFW_RELEASE)
			{
				if (key == GLFW_MOUSE_BUTTON_LEFT)
				{
					HeldObject = NULL;
				}
			} // Held Note, Mouse Button Release
		}

	}
}

bool ScreenEdit::Run(double delta)
{
	GraphMan.isGuiInputEnabled = (EditScreenState != Playing);

	// we're playing the song? run the game
	if (EditScreenState == Playing)
	{
		if (Music->IsStopped())
			startMusic();
		ScreenGameplay::Run(delta);
		RenderObjects(delta);
	}
	else // editing the song? run the editor
	{
		if (CurrentDiff->Measures.size())
		{
			Barline.Run(delta, CurrentDiff->Measures[Measure].Fraction, CurrentFraction);
			if (! (Measure % 2) )
				YLock =  ((float)CurrentFraction / (float)CurrentDiff->Measures[Measure].Fraction) * (float)PlayfieldHeight;
			else
				YLock =  PlayfieldHeight - ((float)CurrentFraction / (float)CurrentDiff->Measures[Measure].Fraction) * (float)PlayfieldHeight;

			YLock += ScreenOffset;
		}

		GhostObject.SetPositionY(YLock);
		GhostObject.SetPositionX(GraphMan.GetRelativeMPos().x);
		
		if ((GhostObject.GetPosition().x-ScreenDifference) > PlayfieldWidth)
			GhostObject.SetPositionX(PlayfieldWidth+ScreenDifference);
		if ((GhostObject.GetPosition().x-ScreenDifference) < 0)
			GhostObject.SetPositionX(ScreenDifference);

		RenderObjects(delta);

		if (CurrentDiff->Measures.size())
		{
			try
			{
				if (Measure > 0)
				{
					for (std::vector<GameObject>::reverse_iterator i = CurrentDiff->Measures.at(Measure-1).MeasureNotes.rbegin(); i != CurrentDiff->Measures.at(Measure-1).MeasureNotes.rend(); i++)
					{	
						if (i->GetPosition().x > ScreenDifference)
							i->Render();
					}
				}

				for (std::vector<GameObject>::reverse_iterator i = CurrentDiff->Measures[Measure].MeasureNotes.rbegin(); i != CurrentDiff->Measures[Measure].MeasureNotes.rend(); i++)
				{
					if (i->GetPosition().x > ScreenDifference)
						i->Render();
				}
			}catch(...) { }
		}

		if (Mode != Select)
			GhostObject.Render();

		std::stringstream info;
		info << "Measure:   " << Measure
			 << "\nFrac:    " << CurrentFraction;
		if (CurrentDiff->Measures.size())
			 info << "\nMaxFrac: " << CurrentDiff->Measures.at(Measure).Fraction;
		info << "\nMode:    ";
		if (Mode == Normal)
			  info << "Normal";
		else if (Mode == Hold)
			info << "Hold";
		else
			info << "Null";
		EditInfo.DisplayText(info.str().c_str(), glm::vec2(512, 600));

		CEGUI::System::getSingleton().renderGUI();
	}

	return Running;
}

void ScreenEdit::AssignFraction(int Measure, int Fraction)
{
	CurrentDiff->Measures.at(Measure).Fraction = Fraction;
	CurrentDiff->Measures.at(Measure).MeasureNotes.resize(CurrentDiff->Measures[Measure].Fraction);

	uint32 count = 0;
	for (std::vector<GameObject>::iterator i = CurrentDiff->Measures.at(Measure).MeasureNotes.begin(); 
		i != CurrentDiff->Measures.at(Measure).MeasureNotes.end(); 
		i++)
	{
		i->MeasurePos = count;
		count++;
	}
}

bool ScreenEdit::measureTextChanged(const CEGUI::EventArgs& param)
{
	if (Utility::IsNumeric(CurrentMeasure->getText().c_str()))
	{
		Measure = boost::lexical_cast<int32>(CurrentMeasure->getText());
		if (Measure < 10000)
		{
			if ((int32)Measure > (((int32)CurrentDiff->Measures.size())-1))
			{
				CurrentDiff->Measures.resize(Measure+1);
			}else
			{
				FracBox->setText( (boost::format("%d\n") % CurrentDiff->Measures[Measure].Fraction).str().c_str() );
			}
		}
	}
	return true;
}

bool ScreenEdit::offsetTextChanged(const CEGUI::EventArgs& param)
{
	try 
	{
		CurrentDiff->Offset = boost::lexical_cast<float>(OffsetBox->getText());
	}catch (...)
	{
		// hmm.. What to do?
	}
	return true;
}

bool ScreenEdit::bpmTextChanged(const CEGUI::EventArgs& param)
{
	try 
	{
		CurrentDiff->Timing[0].Value = boost::lexical_cast<int32>(BPMBox->getText());
	}catch (...)
	{
		// hmm.. What to do?
	}

	return true;
}

bool ScreenEdit::fracTextChanged(const CEGUI::EventArgs& param)
{
	if (Utility::IsNumeric(FracBox->getText().c_str()))
	{
		int32 Frac = boost::lexical_cast<int32>(FracBox->getText());
		if (Frac <= 192)
			AssignFraction(Measure, Frac);
	}

	return true;
}