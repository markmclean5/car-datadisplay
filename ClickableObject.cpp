/* ClickableObject Constructor */
ClickableObject:ClickableObject(void)
{
	isVisible = false;
	isRectangular = false;
	touchEnabled = false;
	cRad = 0;
	cX = 0;
	cY = 0;
	rW = 0;
	rH = 0;
	rblX = 0;
	rblY = 0;
	touchX = 0;
	touchY = 0;
	touchValid = false;
}

/* Protected Members */
void ClickableObject::setCircular(void)
{
	isRectangular = false;
}
void ClickableObject::setCircleCenter(int x, int y)
{
	cX = x;
	cY = y;
}
void ClickableObject::setCircleRadius(int r);
{
	cRad = r;
}
void ClickableObject::setRectangular(void)
{
	isRectangular = true;
}
void ClickableObject::setRectWidthHeight(int w, int h)
{
	rW = w;
	rh = h;
}
void ClickableObject::setRectBottomLeft(int x, int y)
{
	rblX = x;
	rblY = y;
}

/* Public Members */
void ClickableObject::touchEnable(void)
{
	touchEnabled = true;
}
void ClickableObject::touchDisable(void)
{
	touchEnabled = false;
}

void ClickableObject::setVisible(void)
{
	isVisible = true;
}
void ClickableObject::setInvisible(void)
{
	isVisible = false;
}

void ClickableObject::updateTouch(mouse_t input)
{

}


bool ClickableObject::wasTouched(void)
{
	bool wasTouched = false;

	if()
}
};